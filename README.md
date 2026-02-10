Chat con Protocolo DDS (C++)
Este proyecto implementa una comunicación de chat entre nodos utilizando el middleware OpenDDS. La comunicación está configurada mediante RTPS para permitir el descubrimiento automático entre participantes sin necesidad de un servidor central.

Requisitos Previos
Sistema Operativo: Linux (probado en Ubuntu/WSL).
Middleware: OpenDDS instalado y compilado.
Lenguaje: C++.

Compilación
Antes de ejecutar el programa, es necesario generar los archivos de soporte del IDL y compilar el proyecto:

Carga las variables de entorno de OpenDDS: source ~/OpenDDS/setenv.sh
Genera los archivos de compilación con MPC: mwc.pl -type gnuace workspace.mwc
Compila el proyecto: make

Ejecución
Para iniciar un nodo de chat, sigue estos pasos:
Navega a la carpeta del proyecto: cd /mnt/c/dds_chat
Asegúrate de tener el entorno cargado: source ~/OpenDDS/setenv.sh
Ejecuta el nodo utilizando el archivo de configuración RTPS: ./chat_node -DCPSConfigFile rtps.ini
