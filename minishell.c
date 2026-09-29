#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>  
#include <sys/wait.h>   
#include <fcntl.h>     

#define MAX_LINE 1024
#define MAX_ARGS 64
#define DELIM " \t\r\n" // Delimitadores: espacio, tabulación, retorno de carro y salto de línea
#define PATH_MAX_LEN 1024   

// Función auxiliar para procesar redirecciones (< y >) y ejecutar execvp
void execute_single_command(char **cmd_args) {
    char *input_file = NULL;
    char *output_file = NULL;
    int j = 0;

    // Detectar si existen operadores de redirección
    while (cmd_args[j] != NULL) {
        if (strcmp(cmd_args[j], "<") == 0) {
            if (cmd_args[j + 1] == NULL) {
                fprintf(stderr, "minishell: error de sintaxis cerca de '<'\n");
                exit(EXIT_FAILURE);
            }
            input_file = cmd_args[j + 1];
            cmd_args[j] = NULL; // Corta los argumentos
        } 
        else if (strcmp(cmd_args[j], ">") == 0) {
            if (cmd_args[j + 1] == NULL) {
                fprintf(stderr, "minishell: error de sintaxis cerca de '>'\n");
                exit(EXIT_FAILURE);
            }
            output_file = cmd_args[j + 1];
            cmd_args[j] = NULL; // Corta los argumentos
        }
        j++;
    }

    // Redirección de entrada (<)
    if (input_file != NULL) {
        int fd_in = open(input_file, O_RDONLY);
        if (fd_in < 0) {
            perror(input_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    // Redirección de salida (>)
    if (output_file != NULL) {
        int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out < 0) {
            perror(output_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    // Ejecutar programa externo
    execvp(cmd_args[0], cmd_args);
    perror(cmd_args[0]);
    exit(EXIT_FAILURE);
}

int main(void) {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        printf("minishell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\nSaliendo de minishell (EOF detectado)...\n");
            break;
        }

        int i = 0;
        args[i] = strtok(line, DELIM);
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok(NULL, DELIM);
        }

        if (args[0] == NULL) {
            continue;
        }

        // Para Visualizar la Separación
        
        /* printf("[DEBUG] Comando ingresado: %s\n", args[0]);
        for (int j = 1; args[j] != NULL; j++) {
            printf("  [DEBUG] Argumento [%d]: %s\n", j, args[j]);
        } */

        // ------------------------------------ COMANDOS INTERNOS (BUILT-INS) ------------------------------------

        if (strcmp(args[0], "exit") == 0) {
            printf("Saliendo de minishell...\n");
            break;
        }

        if (strcmp(args[0], "pwd") == 0) {
            char cwd[PATH_MAX_LEN];

            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd"); // Muestra el mensaje de error del sistema si falla
            }
            continue; // Volver al prompt sin intentar ejecutar como programa externo
        }

        if (strcmp(args[0], "cd") == 0) {
            char *target_dir = args[1];

            if (target_dir == NULL) {
                target_dir = getenv("HOME");
            }

            if (target_dir != NULL) {
                if (chdir(target_dir) != 0) {
                    perror("cd"); // Informa errores como directorio inexistente o sin permisos
                }
            } else {
                fprintf(stderr, "cd: no se pudo determinar el directorio HOME\n");
            }
            continue;
        }

        // Por ahora, para comandos externos no implementados aún
        /* printf("[DEBUG] Comando externo no soportado todavía: %s\n", args[0]); */


        // --------------------------------------- CON PIPE (|) ---------------------------------------

        int pipe_idx = -1;
        for (int k = 0; args[k] != NULL; k++) {
            if (strcmp(args[k], "|") == 0) {
                pipe_idx = k;
                break;
            }
        }

        if (pipe_idx != -1) {
            args[pipe_idx] = NULL; // Separa el comando izquierdo del derecho
            char **left_cmd = args;
            char **right_cmd = &args[pipe_idx + 1];

            if (left_cmd[0] == NULL || right_cmd[0] == NULL) {
                fprintf(stderr, "minishell: error de sintaxis cerca de '|'\n");
                continue;
            }

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                continue;
            }

            // Primer hijo: ejecuta left_cmd
            pid_t pid1 = fork();
            if (pid1 < 0) {
                perror("fork");
                close(pipefd[0]);
                close(pipefd[1]);
                continue;
            }

            if (pid1 == 0) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
                execute_single_command(left_cmd);
            }

            // Segundo hijo: ejecuta right_cmd
            pid_t pid2 = fork();
            if (pid2 < 0) {
                perror("fork");
                close(pipefd[0]);
                close(pipefd[1]);
                waitpid(pid1, NULL, 0); // !
                continue;
            }

            if (pid2 == 0) {
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                close(pipefd[1]); 
                execute_single_command(right_cmd);
            }

            // --- PROCESO PADRE ---
            // Cierra ambos extremos del pipe en el padre. CRÍTICO para que el lector reciba EOF.
            close(pipefd[0]);
            close(pipefd[1]);

            // Esperar a ambos hijos
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        } 

        // --------------------------------------- SIN PIPE (|) ---------------------------------------
        else {
            pid_t pid = fork();

            if (pid < 0) {
                perror("fork");
            } 
            else if (pid == 0) {
                execute_single_command(args);
            } 
            else {
                int status;
                if (waitpid(pid, &status, 0) == -1) {
                    perror("waitpid");
                }
            }
        }
    }

    return 0;
}