#include "redismodule.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int hashCopyField_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
//    hash_command.copyField key srcfield newField
    
    RedisModule_AutoMemory(ctx);
    
    if (argc != 4) return RedisModule_WrongArity(ctx);
    
    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1],
                                              REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_HASH &&
        type != REDISMODULE_KEYTYPE_EMPTY)
    {
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    
    
//    RedisModuleKey *newFieldKey =
//    RedisModule_OpenKey(ctx, argv[2], REDISMODULE_WRITE);
//    RedisModule_DeleteKey(newFieldKey);
    
   
    RedisModuleString *oldval;
    RedisModule_HashGet(key,REDISMODULE_HASH_NONE,argv[2],&oldval,NULL);
    if (oldval) {
        RedisModule_HashSet(key,REDISMODULE_HASH_NONE,argv[3],oldval,NULL);
    }
    RedisModule_ReplyWithLongLong(ctx,oldval != NULL);
    return REDISMODULE_OK;
    
    
    
}


int hashANDquery_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    // hashkey word1 hash_command.AND word2
    
    // hash_command.AND word1 word2 hashkey
    
    // myhash tarang hash_command.AND ziroh
    
    RedisModule_AutoMemory(ctx);
    
    if (argc != 4) return RedisModule_WrongArity(ctx);
    
    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[4],
                                              REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_HASH &&
        type != REDISMODULE_KEYTYPE_EMPTY)
    {
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    
//    for loop to find the fields and store them in an array
//    loop through all the fields and see if the hash contains both the words. For ever field check if it has either word1 or word2
//    if it does contain a word then store in an array. If both words are in the array then reply "true".
    
    
    RedisModuleString *field1;
    RedisModule_HashGet(key,REDISMODULE_HASH_NONE,field1);
    
    const char *f1 = RedisModule_StringPtrLen(field1,NULL);
    
    
    
    RedisModule_ReplyWithSimpleString(ctx, f1);
    
    
    RedisModule_ReplicateVerbatim(ctx);
    
    return REDISMODULE_OK;
    
}

int hashInsert_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    //  hash_command.insert hashkey KeyWord FileName
    
//    hash_command.insert myhash "Tarang"  names.txt
    
    if (argc != 4) return RedisModule_WrongArity(ctx);
    
    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1],
                                              REDISMODULE_READ|REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_HASH &&
        type != REDISMODULE_KEYTYPE_EMPTY)
    {
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    
    RedisModule_HashSet(key,REDISMODULE_HASH_NONE, argv[2],argv[3],NULL);
    
    
}










int RedisModule_OnLoad(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    if(RedisModule_Init(ctx , "hash_command" , 1 , REDISMODULE_APIVER_1) == REDISMODULE_ERR){
        return REDISMODULE_ERR; }
    
    if(RedisModule_CreateCommand(ctx, "hash_command.copyField" , hashCopyField_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR) { return REDISMODULE_ERR;}
    
     if(RedisModule_CreateCommand(ctx, "hash_command.AND" , hashANDquery_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR) { return REDISMODULE_ERR;}
    
     if(RedisModule_CreateCommand(ctx, "hash_command.insert" , hashInsert_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR) { return REDISMODULE_ERR;}
    
     return REDISMODULE_OK;
}
