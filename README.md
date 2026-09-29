make
./minishell
    ---------------------Comandos Internos (cd, pwd, exit)---------------------
    
    pwd
    cd /tmp
    pwd
    cd carpeta_inventada_123
    cd
    pwd
    Verificación: cd /tmp debe cambiar tu directorio, el comando con la carpeta inventada debe imprimir un error con perror("cd") sin cerrar el programa, y cd solo debe devolverte a tu directorio principal (HOME).

    ---------------------Comandos Externos (fork, execvp, waitpid)---------------------
    ls
    ls -la
    echo "Probando ejecucion externa"
    comando_que_no_existe
    Verificación: Los comandos estándar deben mostrar su salida en pantalla y la minishell debe esperar a que terminen antes de volver a mostrar minishell>. El comando inexistente debe mostrar error con perror y seguir corriendo.


    ---------------------Redirecciones (< y >)---------------------
    ls -l > archivo_prueba.txt
    cat < archivo_prueba.txt
    cat < archivo_inexistente.txt
    Verificación: ls -l > ... no debe mostrar nada en pantalla, pero debe crear archivo_prueba.txt. El comando cat < ... debe leer su contenido. El archivo inexistente debe mostrar un error de apertura.


    ---------------------Pipe Simple (|) y combinación---------------------
    ls -l | grep txt
    cat minishell.c | grep include
    ls -l | grep txt > resultado_pipe.txt
    cat < resultado_pipe.txt
    Verificación: Debe filtrar las salidas adecuadamente sin congelarse la terminal.


    ---------------------Salida de la minishell---------------------
    exit
    (O presiona la combinación de teclas Ctrl + D en una línea vacía para probar la detección de EOF).
make clean