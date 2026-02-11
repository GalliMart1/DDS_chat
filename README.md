# Chat P2P con Protocolo DDS (C++)
  Este proyecto implementa una comunicación de chat en tiempo real entre nodos distribuidos utilizando el middleware **OpenDDS**.
  La arquitectura es **Peer-to-Peer (P2P)** utilizando el protocolo de interoperabilidad **RTPS** (Real-Time Publish-Subscribe). Esto permite que los participantes se descubran y comuniquen automáticamente en la red local sin necesidad de un servidor centralizado ni un repositorio de información (DCPSInfoRepo).

### Requisitos Previos
  Para compilar y ejecutar este proyecto, necesitas tener instalado y configurado el entorno de **OpenDDS** en un entorno Linux (probado en **Ubuntu/WSL**).

Asegúrate de tener cargadas las variables de entorno antes de empezar: 
```bash
source ~/OpenDDS/setenv.sh
```
### Compilación
  Sigue estos pasos para generar los binarios y archivos de soporte desde cero:

#### 1. Navegar al directorio del proyecto:
``` bash
cd dds_chat
```

#### 2. Generar el Makefile: 
  Utilizamos MPC (Make Project Creator) para generar los archivos de compilación basados en el workspace y el tipo de sistema (GNU Make).
``` bash
$ACE_ROOT/bin/mwc.pl -type gnuace
```

#### 3. Compilar:
``` bash
make
```
Esto generará el código base del IDL y un ejecutable principal llamado chat_node.

### Ejecución
  Para probar la comunicación, necesitas simular al menos dos nodos (pueden ser dos terminales en la misma PC o dos PCs diferentes en la misma red LAN).

#### Paso 1: Configuración de Red.
  El proyecto utiliza el archivo rtps.ini para la configuración de descubrimiento e interoperabilidad. Asegúrate de que este archivo esté en la misma carpeta que el ejecutable.

#### Paso 2: Iniciar Nodos
Terminal 1 (Usuario A):
``` bash
./chat_node -DCPSConfigFile rtps.ini
```
Terminal 2 (Usuario B):
``` bash
./chat_node -DCPSConfigFile rtps.ini
```
Nota: Al arrancar, el protocolo RTPS iniciará el handshake automático para descubrir otros participantes en el dominio.

### Uso
  El programa funciona simultáneamente como Publicador (DataWriter) y Suscriptor (DataReader).
  
- Envío: Escribe tu mensaje en la consola y presiona Enter para enviarlo al tópico global.
- Recepción: Los mensajes enviados por otros nodos aparecerán automáticamente en tu pantalla con el identificador del remitente.
  
### Estructura del Proyecto
- **Messenger.idl:** Definición de la estructura de datos (IDL). Aquí se define el mensaje de chat y las claves (keys) de los participantes.
- **Chat_Node.cpp:** Código fuente principal. Contiene la lógica de inicialización del participante, el DataWriter y el DataReaderListener.
- **rtps.ini:** Archivo de configuración crítica para OpenDDS. Define el transporte sobre UDP/Multicast y los parámetros de descubrimiento P2P.
- **workspace.mwc / chat.mpc:** Archivos de definición del proyecto para el sistema de compilación MPC.

 

