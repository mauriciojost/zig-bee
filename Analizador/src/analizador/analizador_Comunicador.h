/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class analizador_Comunicador */

#ifndef _Included_analizador_Comunicador
#define _Included_analizador_Comunicador
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     analizador_Comunicador
 * Method:    iniciar
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_analizador_Comunicador_iniciar
  (JNIEnv *, jobject);

/*
 * Class:     analizador_Comunicador
 * Method:    enviar
 * Signature: (B)V
 */
JNIEXPORT void JNICALL Java_analizador_Comunicador_enviar
  (JNIEnv *, jobject, jbyte);

/*
 * Class:     analizador_Comunicador
 * Method:    recibir
 * Signature: ()B
 */
JNIEXPORT jbyte JNICALL Java_analizador_Comunicador_recibir
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
