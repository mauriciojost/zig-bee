#define __WINDOWS_COM__
#include <jni.h>
#include <stdio.h>
#include "analizador_Comunicador.h"
#include "com/serial.h"


DCB OldConf;
HANDLE fd;
#define LARGO_BUFFER 5000
char buffer[LARGO_BUFFER];

enum BUFF_ESTADO {LLENO=1, VACIO=0};
BUFF_ESTADO b_estado;

//extern "C" void __stdcall inicia(); // Linea extra�a...
//extern "C" char __stdcall lee(); // Linea extra�a...
//extern "C" void __stdcall escribe(char); // Linea extra�a...


HANDLE inicializar_serie(char* puerto){
    HANDLE fd;
    char array[3];
    int i;
    
    for(i=0;i<LARGO_BUFFER;i++){
        buffer[i]=0;
    }
    b_estado = LLENO;
    fd=Open_Port(puerto);           // Abre el puerto serie.
    OldConf=Get_Configure_Port(fd); // Guarda la configuracion del puerto.
    Configure_Port(fd,B9600,"8N1"); // Configura el puerto serie.
    
    return fd;
}

void finalizar_serie(HANDLE fd){
  Set_Configure_Port(fd,OldConf); // Restituye la antigua configuracion.
  Close_Port(fd);                 // Cierra el puerto serie.
}


JNIEXPORT void JNICALL Java_analizador_Comunicador_enviar(JNIEnv *env, jobject obj, jbyte c){
    char array[3];
    
    // borrar de aca
    
    char caracter='9';
    unsigned int i=0;
    // hasta aca
    
    printf("-%c",(jbyte)c);
    array[0] = c;							// enviarla por puerto serie.
    Write_Port(fd,array,1);    // Escribe en el puerto serie.
    
    // y de aca
    for(i=0;i<100000;i++){
        array[0]=c;
    }
    
        i=0;
    if (Kbhit_Port(fd)!=0){    // BLOQUEANTE O NO????
        printf("Hay algo...\n");
        while(caracter!='\n'){
            Read_Port(fd,array,1);			// ARREGLAR!!! PUEDE QUE DEVUELVA BASURA!!!
            caracter = (jbyte)array[0];
            buffer[i++] = caracter;
        }
        buffer[i-1]=0; // Finaliza el string.
        b_estado=LLENO;
        printf("\nEsto fue recibido en C luego de enviar '%s'\n",buffer);
    }
    // hasta aca
        

}






JNIEXPORT jstring JNICALL Java_analizador_Comunicador_recibir(JNIEnv *env, jobject obj){
    jbyte retorno;
    char array[3];
    char caracter='9';
    unsigned int i = 0;
    
    if (b_estado==LLENO){
        printf("El buffer estaba lleno al leer.\n");
        b_estado = VACIO;
        
    }else{
        printf("El buffer estaba vacio al leer.\n");
        while(caracter!='\n'){
            Read_Port(fd,array,1);			// ARREGLAR!!! PUEDE QUE DEVUELVA BASURA!!!
            caracter = (jbyte)array[0];
            buffer[i++] = caracter;
            printf("'%c'",caracter);
        }
        buffer[i-1]=0;
    }
    printf("Retorno para la lectura (C): '%s'.\n",buffer);
    
    return env->NewStringUTF(buffer);
}



JNIEXPORT void JNICALL Java_analizador_Comunicador_iniciar(JNIEnv *env, jobject obj){
    fd = inicializar_serie("COM1");
}


JNIEXPORT void JNICALL Java_analizador_Comunicador_finalizar(JNIEnv *env, jobject obj){
    finalizar_serie(fd);
}






/*
 JNIEXPORT void JNICALL Java_Comunicador_enviarComando(JNIEnv *env, jobject obj, jstring comando){
		int suma;
    const char *str = env->GetStringUTFChars(comando, 0);
    printf("Estamos en C...\n");
 		suma = Addup (1, 1, 1);
    printf("Comando para el PIC: %s (suma %d)\n", str,suma);

}


JNIEXPORT jstring JNICALL Java_Comunicador_recibirComando(JNIEnv *env, jobject obj){
    const char retorno[] = "Enviado desde el PIC...";
    return env->NewStringUTF(retorno);
}

 
 */