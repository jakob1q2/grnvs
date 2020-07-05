#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "GRNVS_RAW.h"
#include "raw.h"
#include "hexdump.h"
#include "checksums.h"

#define min(x, y) (x) < (y) ? (x) : (y)

JNIEXPORT jint JNICALL Java_GRNVS_1RAW_getSocket
	(JNIEnv * env, jobject obj, jstring dev, jint level)
{
	const char * device = (*env)->GetStringUTFChars(env, dev, 0);
	int sock = grnvs_open(device, level);
	(*env)->ReleaseStringUTFChars(env, dev, device);
	if(sock < 0)
		(*env)->ThrowNew(env,
			(*env)->FindClass(env, "java/lang/Exception"),
			strerror(errno));
	return sock;
}

JNIEXPORT jint JNICALL Java_GRNVS_1RAW_write_1
	(JNIEnv * env, jobject obj, jint fd, jbyteArray buffer, jint length)
{
	char buf[1514];
	(*env)->GetByteArrayRegion(env, buffer, 0, length, buf);
	return grnvs_write(fd, buf, length);
}

JNIEXPORT void JNICALL Java_GRNVS_1RAW_hexdump_1
  (JNIEnv * env, jclass class, jbyteArray buffer, jint length)
{
	char buf[1514];
	(*env)->GetByteArrayRegion(env, buffer, 0, length, buf);
	hexdump(buf, length);
}

JNIEXPORT jint JNICALL Java_GRNVS_1RAW_read_1
	(JNIEnv * env, jobject obj, jint fd, jbyteArray buffer, jobject time)
{
	char buf[1514];
	int length;
	int timeout;
	int * tp = NULL;
	jclass tclass;
	jmethodID get;
	jmethodID set;
	if(time) {
		tclass = (*env)->GetObjectClass(env, time);
		get = (*env)->GetMethodID(env, tclass, "getTimeout", "()I");
		set = (*env)->GetMethodID(env, tclass, "setTimeout", "(I)V");
		if(get == NULL || set == NULL) {
			fprintf(stderr, "Could not get one of the methods\n");
			return -1;
		}
		timeout = (int)(long long)(*env)->CallObjectMethod(env, time, get);
		tp = &timeout;
	}

	memset(buf, 0, sizeof(buf));
	length = min(sizeof(buf), (*env)->GetArrayLength(env, buffer));
	length = grnvs_read(fd, buf, length, tp);
	if(time)
		(*env)->CallObjectMethod(env, time, set, timeout);
	(*env)->SetByteArrayRegion(env, buffer, 0, length, buf);
	return length;
}

JNIEXPORT jint JNICALL Java_GRNVS_1RAW_close_1
	(JNIEnv * env, jobject obj, jint fd)
{
	return grnvs_close(fd);
}

JNIEXPORT jbyteArray JNICALL Java_GRNVS_1RAW_mac_1
	(JNIEnv * env, jobject obj, jint fd)
{
	const uint8_t * mac = grnvs_get_hwaddr(fd);
	jbyteArray result;
	if(!mac)
		return NULL;
	result = (*env)->NewByteArray(env, 6);
	(*env)->SetByteArrayRegion(env, result, 0, 6, (const jbyte*)mac);
	return result;
}

JNIEXPORT jbyteArray JNICALL Java_GRNVS_1RAW_ip_1
	(JNIEnv * env, jobject obj, jint fd)
{
	const unsigned char * addr;
	struct in_addr ip;
	jbyteArray result;

	ip = grnvs_get_ipaddr(fd);
	addr = (unsigned char *)&ip;
	result = (*env)->NewByteArray(env, 4);
	(*env)->SetByteArrayRegion(env, result, 0, 4, addr);
	return result;
}

JNIEXPORT jbyteArray JNICALL Java_GRNVS_1RAW_ip6_1
  (JNIEnv *env, jobject obj , jint fd)
{
	const struct in6_addr * ip;
	jbyteArray result;

	ip = grnvs_get_ip6addr(fd);
	result = (*env)->NewByteArray(env, 16);
	(*env)->SetByteArrayRegion(env, result, 0, 16, (const jbyte*)ip);
	return result;
}


JNIEXPORT jbyteArray JNICALL Java_GRNVS_1RAW_checksum_1
  (JNIEnv * env, jclass class, jbyteArray hdr, jint hdr_offset,
  jbyteArray payload, jint payload_offset, jint payload_length)
{
	uint8_t hdrbuffer[40];
	uint8_t payloadbuffer[2048];
	uint16_t cksum;
	jbyteArray result;

	(*env)->GetByteArrayRegion(env, hdr, hdr_offset, 40, hdrbuffer);
	(*env)->GetByteArrayRegion(env, payload, payload_offset,
	                           payload_length, payloadbuffer);
	cksum = icmp6_checksum((struct ip6_hdr*)hdrbuffer, payloadbuffer,
	                       payload_length);
	result = (*env)->NewByteArray(env, 2);
	(*env)->SetByteArrayRegion(env, result, 0, 2, (const jbyte*)&cksum);
	return result;
}
