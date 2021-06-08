#ifndef __CONSUMER_C_H__
#define __CONSUMER_C_H__

#ifndef __CONSUMER_C_INTERFACE_IMPLEMENTATION__
typedef void* asapoConsumer;
typedef void* asapoSourceCredentials;
typedef void* asapoError;
struct asapoErrorBuffer {
  asapoError error;
};
typedef void* asapoMessageMeta;
typedef void* asapoMessageData;

#endif

asapoConsumer asapoCreateConsumer(const char* server_name,
				  const char* source_path,
				  _Bool has_filesysytem,
				  asapoSourceCredentials source,
				  asapoErrorBuffer* error);
asapoMessageMeta asapoCreateMessageMeta();
void asapoDeleteMessageMeta(asapoMessageMeta* meta);

#endif
