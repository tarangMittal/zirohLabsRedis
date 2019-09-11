#include "redismodule.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

//int ListExtendFilter_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc);
//int ListExtendFilter_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc);


static RedisModuleDict *Keyspace;

int ListExtendSort_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    // LIST_EXTEND.SORT source_list sorted_list
    if(argc !=3){
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    //open field/value list keys
    RedisModuleKey *sourceListKey =
    RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int sourceListKeyType = RedisModule_KeyType(sourceListKey);
    
    if (sourceListKeyType != REDISMODULE_KEYTYPE_LIST &&
        sourceListKeyType != REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_CloseKey(sourceListKey);
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    
    //open sorted key
    
    RedisModuleKey *sortedListKey =
                RedisModule_OpenKey(ctx, argv[2], REDISMODULE_WRITE);
    RedisModule_DeleteKey(sortedListKey);
    
    //get length of source list
    size_t sourceListLength = RedisModule_ValueLength(sourceListKey);
    
    if (sourceListLength == 0) {
        RedisModule_ReplyWithLongLong(ctx, 0L);
        return REDISMODULE_OK;
    }
    
    // the number of added elements to the sorted_list
    //size_t added = 0;
    
    
    //sorting part
    
    long long sortedArr[10];
    int count =0;
    long long temp;
    
    for (size_t pos = 0; pos < sourceListLength; pos++) {
        RedisModuleString *ele =
        RedisModule_ListPop(sourceListKey, REDISMODULE_LIST_TAIL);
        //RedisModule_ListPush(sourceListKey,REDISMODULE_LIST_TAIL,ele);
        
         long long val;
        if (RedisModule_StringToLongLong(ele, &val) == REDISMODULE_OK){
            sortedArr[count] = val;
            
        }
        count++;
        
    }
    
    for(int i=0 ; i< count ; i++){
        for(int j=0; j< count -1;j++){
            if(sortedArr[j] > sortedArr[j+1]){
                temp = sortedArr[j+1];
                sortedArr[j+1] = sortedArr[j];
                sortedArr[j] = temp;
            }
        }
    }
    
    for(int i=0 ; i<count ; i++){
        
        // RedisModule_Log(ctx, "notice", "element within range %d %d %d",
        // lowerLimit, val, upperLimit );
        RedisModuleString *newele = RedisModule_CreateStringFromLongLong(ctx, sortedArr[i]);
        
        
        (RedisModule_ListPush(sortedListKey, REDISMODULE_LIST_TAIL,newele));
        
    }
    
    RedisModule_ReplyWithLongLong(ctx, count);
    
    RedisModule_ReplicateVerbatim(ctx);
    
    return REDISMODULE_OK;
    
}

int ListExtendFilter_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
	// LIST_EXTEND.FILTER source_list destination_list low_value high_value

	if(argc !=5) {
	return RedisModule_WrongArity(ctx);
	}
	RedisModule_AutoMemory(ctx);

	// Open field/value list keys
  	RedisModuleKey *sourceListKey =
      	RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
 	 int sourceListKeyType = RedisModule_KeyType(sourceListKey);

	if (sourceListKeyType != REDISMODULE_KEYTYPE_LIST &&
      	sourceListKeyType != REDISMODULE_KEYTYPE_EMPTY) {
    		RedisModule_CloseKey(sourceListKey);
    		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
 	 }
	// Open destination key
 	 RedisModuleKey *destinationListKey =
      	       RedisModule_OpenKey(ctx, argv[2], REDISMODULE_WRITE);
 	 RedisModule_DeleteKey(destinationListKey);
	
	// Get length of list
 	 size_t sourceListLength = RedisModule_ValueLength(sourceListKey);

	if (sourceListLength == 0) {
   	 RedisModule_ReplyWithLongLong(ctx, 0L);
   	 return REDISMODULE_OK;
 	 }
	
	 char strLowerLimit[] = "-inf";
 	 char strUpperLimit[] = "+inf";
 	 RedisModuleString *expectedMinusInf, *expectedPlusInf;
 	 long long lowerLimit;
 	 long long upperLimit;	
 	 int lowerLimitOk = 0;
	 int upperLimitOk = 0;

	 expectedMinusInf = RedisModule_CreateString(ctx, strLowerLimit, 4);
 	 expectedPlusInf = RedisModule_CreateString(ctx, strUpperLimit, 4);

	// check if argv[3] is -inf
	  if (RedisModule_StringCompare(argv[3], expectedMinusInf) == 0) {
	    lowerLimit = LONG_MIN;
	    lowerLimitOk = 1;
	  } else {
	    if (RedisModule_StringToLongLong(argv[3], &lowerLimit) == REDISMODULE_OK)
	      lowerLimitOk = 1;
	  } 

	// check if argv[4] is +inf
	  if (RedisModule_StringCompare(argv[4], expectedPlusInf) == 0) {
	    upperLimit = LONG_MAX;
	    upperLimitOk = 1;
	  } else {
	    if (RedisModule_StringToLongLong(argv[4], &upperLimit) == REDISMODULE_OK)
	      upperLimitOk = 1;
	  }


	// the number of added elements to the destination_list
	  size_t added = 0;
	  // loop trough source_list

	  for (size_t pos = 0; pos < sourceListLength; pos++) {
	    RedisModuleString *ele =
		RedisModule_ListPop(sourceListKey, REDISMODULE_LIST_TAIL);
	    RedisModule_ListPush(sourceListKey, REDISMODULE_LIST_HEAD, ele);

	    long long val;

	    // convert string to long long
	    if ((RedisModule_StringToLongLong(ele, &val) == REDISMODULE_OK) &&
		lowerLimitOk == 1 && upperLimitOk == 1) {
	      if (val >= lowerLimit && val <= upperLimit) {

		// RedisModule_Log(ctx, "notice", "element within range %d %d %d",
		// lowerLimit, val, upperLimit );
		RedisModuleString *newele =
		    RedisModule_CreateStringFromLongLong(ctx, val);

		// push to destination_list
		if (RedisModule_ListPush(destinationListKey, REDISMODULE_LIST_HEAD,
					 newele) == REDISMODULE_ERR) {
		  return REDISMODULE_ERR;
		}
		added++;
	      }
	    }
	  }

	  RedisModule_ReplyWithLongLong(ctx, added);
	// propagate the command to the slaves and AOF file exactly as it was called
	  RedisModule_ReplicateVerbatim(ctx);

	 return REDISMODULE_OK;
}


int ListExtendSplitString_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    
//        printf("hello");
//    pass a string like "The_quick_brown_fox_jumps_over_the_lazy_dog"
    
//    list_extend.splitString largeString wordsList
    
    if(argc !=3) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);
    
    // Open field/value list keys
    RedisModuleKey *largeStringKey =
    RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int largeStringKeyType = RedisModule_KeyType(largeStringKey);
    
    if (largeStringKeyType != REDISMODULE_KEYTYPE_LIST &&
        largeStringKeyType != REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_CloseKey(largeStringKey);
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    // Open wordslist key
    RedisModuleKey *wordsListKey =
    RedisModule_OpenKey(ctx, argv[2], REDISMODULE_WRITE);
    RedisModule_DeleteKey(wordsListKey);
    
    size_t largeStringLength = RedisModule_ValueLength(largeStringKey);

    
    if (largeStringLength == 0) {
        RedisModule_ReplyWithLongLong(ctx, 0L);
        return REDISMODULE_OK;
    }
    
//    if(largeStringKeyType==REDISMODULE_KEYTYPE_STRING){
//        size_t len,j;
//        char *s = RedisModule_StringDMA(largeStringKey,&len,REDISMODULE_WRITE);
//        printf("the sring is: %s ",s);
//      //  RedisModule_ReplyWithSimpleString(ctx,s);
//
//        char *p;
//        p = strtok(s,"_");
//
//    }
    
//    RedisModule_Log(ctx, "notice", "starting execution");
    
    RedisModuleString *ele = RedisModule_ListPop(largeStringKey, REDISMODULE_LIST_HEAD);
    RedisModule_ListPush(largeStringKey , REDISMODULE_LIST_HEAD , ele);
    
//    typedef struct RedisModuleString RedisModuleString;
    
    
    // "The_quick_brown_fox_jumps_over_the_lazy_dog" ->
    // The , quick , brown , fox , jumps , over , the , lazy , dog
    
    
//
    
//    wordsArr = (char*)malloc(50* sizeof(char));

  
    int i=0;
    int count =0;

    char* str;
    str = (char*)malloc(1000);
//

    const char *element = RedisModule_StringPtrLen(ele, &largeStringLength);
    
    strcpy(str,element);
    
    
   char *p = strtok(str, "_");
    
    
//
//    while(p!=NULL){
//        wordsArr[i++] = p;
//        p = strtok(NULL, "_");
//        count++;
////
//////        RedisModule_ListPush(wordsListKey,REDISMODULE_LIST_HEAD,token);
//    }
    
    
    while(p!=NULL){
         RedisModuleString *finalWord  = RedisModule_CreateString(ctx, p, strlen(p));
        RedisModule_ListPush(wordsListKey , REDISMODULE_LIST_TAIL , finalWord);
        p = strtok(NULL, "_");
    }
    

//
//    for(int j=0; j<count;j++ ){
//        char *word = wordsArr[j];
//        RedisModuleString *finalWord  = RedisModule_CreateString(ctx, word, strlen(word));
//        RedisModule_ListPush(wordsListKey , REDISMODULE_LIST_TAIL , finalWord);
//
//    }
    
    RedisModule_ReplyWithSimpleString(ctx, "finished");
//
//    RedisModule_ReplyWithLongLong(ctx, 0L);
    
    
    RedisModule_ReplicateVerbatim(ctx);
    
    return REDISMODULE_OK;
    
    
    
}

int ListExtendLexiSortString_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
//    list_extend.lexiSortStrings sourceList  lexiSortedList
    
    if(argc !=3) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);
    
    
    
    // Open field/value list keys
    RedisModuleKey *sourceListKey =
    RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int sourceListKeyType = RedisModule_KeyType(sourceListKey);
    
    if (sourceListKeyType != REDISMODULE_KEYTYPE_LIST &&
        sourceListKeyType != REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_CloseKey(sourceListKey);
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
  
    RedisModuleKey *lexiSortedListKey =
    RedisModule_OpenKey(ctx, argv[2], REDISMODULE_WRITE);
    RedisModule_DeleteKey(lexiSortedListKey);
    
    size_t sourceListLength = RedisModule_ValueLength(sourceListKey);
    
    
    if (sourceListLength == 0) {
        RedisModule_ReplyWithLongLong(ctx, 0L);
        return REDISMODULE_OK;
    }
    
    
    
    
    char sortedArr[10][50];
//    sortedArr = (char*)malloc(1000);

    char temp[50];
//    temp = (char*)malloc(1000);
    
//
    for (size_t pos = 0; pos < sourceListLength; pos++) {
        RedisModuleString *ele = RedisModule_ListPop(sourceListKey, REDISMODULE_LIST_TAIL);
        const char *element = RedisModule_StringPtrLen(ele, NULL);
        strcpy(sortedArr[pos],element);
    }
    
    
//
    for(int i=0; i<sourceListLength; ++i)
        for(int j=i+1; j<sourceListLength ; ++j)
        {
            if(strcmp(sortedArr[i], sortedArr[j])>0)
            {
                strcpy(temp, sortedArr[i]);
                strcpy(sortedArr[i], sortedArr[j]);
                strcpy(sortedArr[j], temp);
            }
        }
    
   
    
//
//
    for(int q=0; q<sourceListLength;q++){
        RedisModuleString *word  = RedisModule_CreateString(ctx, sortedArr[q], strlen(sortedArr[q]));
        RedisModule_ListPush(lexiSortedListKey , REDISMODULE_LIST_TAIL , word);
    }

    RedisModule_ReplyWithSimpleString(ctx, "finished");
    
    
    RedisModule_ReplicateVerbatim(ctx);
    
    return REDISMODULE_OK;
    
    
}


int Keys_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    // list_extend.keys keys
    
    if(argc !=2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);
    
    
    
    // Open field/value list keys
    RedisModuleKey *key =
    RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int keyType = RedisModule_KeyType(key);
    
    if (keyType != REDISMODULE_KEYTYPE_LIST &&
        keyType != REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_CloseKey(key);
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    
    size_t length = RedisModule_ValueLength(key);
    
    
    const char *s = RedisModule_StringDMA(key,&length,REDISMODULE_WRITE );
    
    RedisModule_ReplyWithSimpleString(ctx, s);
    
    
    RedisModule_ReplicateVerbatim(ctx);
    
    return REDISMODULE_OK;
    
    
}

int ListInsert_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    // list_extend.insert list
    
    if(argc !=2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);
    
    RedisModuleKey *key =
    RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int keyType = RedisModule_KeyType(key);
    
    if (keyType != REDISMODULE_KEYTYPE_LIST &&
        keyType != REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_CloseKey(key);
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    
    char ele[][30] = {"file.txt" , "firstnames.pdf" , "names.pdf"};
    
    int rowNum = sizeof(ele)/sizeof(ele[0]);
    
    for(int i=0; i<rowNum;i++){
        RedisModuleString *element = RedisModule_CreateString(ctx, ele[i],strlen(ele[i]));
        RedisModule_ListPush(key,REDISMODULE_LIST_TAIL,element);
        RedisModule_DictSet(Keyspace,argv[1],element);
       // RedisModule_RetainString(NULL,element);

    }
     RedisModule_ReplyWithSimpleString(ctx, "finished");
//
//    RedisModule_ReplyWithSimpleString(ctx, "finished");
//
    
    RedisModule_ReplicateVerbatim(ctx);
    
    return REDISMODULE_OK;
    
    
    
}


int ListFetchKeyVal_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    
    RedisModule_Log(ctx,"notice","fetching the key and values");
    
    // list_extend.fetchkeyVal keyname
    
    if(argc !=2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);
    
    // get all keys and store in array
    
//
//    if (RedisModule_IsKeysPositionRequest(ctx)) {
//        RedisModule_KeyAtPos(ctx,1);
//        RedisModule_KeyAtPos(ctx,2);
//
//    }
//
//    int count = ctx->flags;
//
//    RedisModule_ReplyWithLongLong(ctx,count);
    
    
    RedisModuleKey *key =
    RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int keyType = RedisModule_KeyType(key);
    
    if (keyType != REDISMODULE_KEYTYPE_LIST &&
        keyType != REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_CloseKey(key);
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
     size_t length = RedisModule_ValueLength(key);
    long long replylen = 0;
     char temp[length][50] ;
    
    RedisModule_ReplyWithArray(ctx,REDISMODULE_POSTPONED_ARRAY_LEN);
    for(int count =0; count<length;count++){
        RedisModuleString *val = RedisModule_ListPop(key,REDISMODULE_LIST_TAIL);
        const char *ele = RedisModule_StringPtrLen(val,NULL);
        strcpy(temp[count],ele);
        RedisModule_ReplyWithStringBuffer(ctx,ele,strlen(ele));
        replylen++;
    }
    
    for(int i=0; i<length;i++){
        RedisModuleString *element = RedisModule_CreateString(ctx,temp[i],strlen(temp[i]));
        RedisModule_ListPush(key,REDISMODULE_LIST_HEAD, element);
    }
    
    RedisModule_ReplySetArrayLength(ctx,replylen);

    
    RedisModule_ReplicateVerbatim(ctx);
    
    return REDISMODULE_OK;
    // get the values for each key
    
    
}



int listOfKeys_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    // list_extend.keyRange
    
    if(argc !=1) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);
    
//    RedisModuleKey *key =
//    RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
//    int keyType = RedisModule_KeyType(key);
//
//    if (keyType != REDISMODULE_KEYTYPE_LIST &&
//        keyType != REDISMODULE_KEYTYPE_EMPTY) {
//        RedisModule_CloseKey(key);
//        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
//    }
    
//    long long count;
//    if (RedisModule_StringToLongLong(argv[3],&count) != REDISMODULE_OK) {
//        return RedisModule_ReplyWithError(ctx,"ERR invalid count");
//    }
    
    
    
//    RedisModuleDictIter *iter = RedisModule_DictIteratorStart(Keyspace,">=",argv[1]);
    RedisModuleDictIter *iter = RedisModule_DictIteratorStartC(Keyspace,"^",NULL,0);
    
    
   
    
    
    char *bufKey;
    size_t keylen;
    long long replylen = 0;
    RedisModule_ReplyWithArray(ctx,REDISMODULE_POSTPONED_ARRAY_LEN);


    while((bufKey = RedisModule_DictNextC(iter,&keylen,NULL))!=NULL){
//       if (RedisModule_DictCompare(iter,"<=",argv[2]) == REDISMODULE_ERR)
//        break;        if(replylen >= 4){ break; }
        
//        while(replylen < 4){
        
//            bufKey = RedisModule_DictNextC(iter,&keylen,NULL);
        
//            RedisModule_Log(ctx,"notice","the key is  %s and its length is %d", bufKey , keylen);
        
//            *bufKey = '\0';
            
//        RedisModule_Log(ctx,"notice","the key is ruchi and its length is %d", strlen("ruchi"));
        
        RedisModuleString *keyString = RedisModule_CreateString(ctx,bufKey,keylen);
        const char *keysChar = RedisModule_StringPtrLen(keyString,NULL);
        
//        for(int i=0;i<3;i++){
//             RedisModuleString *val = RedisModule_DictGet(Keyspace,keyString,NULL);
//             const char *valPointer = RedisModule_StringPtrLen(val,NULL);
//            RedisModule_Log(ctx,"notice","the value is %s", valPointer);
//
//        }
        
//
//
//
//
//
//       strcat(bufKey,valPointer);

          RedisModule_ReplyWithStringBuffer(ctx,keysChar,keylen);
        replylen++;

        }
    

    RedisModule_ReplySetArrayLength(ctx,replylen);

    RedisModule_DictIteratorStop(iter);
    
//    RedisModule_ReplyWithSimpleString(ctx, "finished");
        RedisModule_ReplicateVerbatim(ctx);
    
    return REDISMODULE_OK;
    
    
}



int getKeys_RedisCommand(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    // list_extend.getKeys *
    
    if(argc !=1) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);
    
     size_t replylen = 0;
    
   RedisModule_ReplyWithArray(ctx,REDISMODULE_POSTPONED_ARRAY_LEN);

//    do{
//
        RedisModuleCallReply *reply = RedisModule_Call(ctx,"KEYS","c","*");
//        RedisModuleCallReply *cr_keys = RedisModule_CallReplyArrayElement(reply,1);
//        size_t items = RedisModule_CallReplyLength(cr_keys);
//
//        for(size_t i=0; i<items;i++)
//        {
//            RedisModuleCallReply *ele = RedisModule_CallReplyArrayElement(reply,i);
//            RedisModule_ReplyWithCallReply(ctx,ele);
//            replylen++;
//        }
//
//    }
//
    size_t items = RedisModule_CallReplyLength(reply);
    
    for(size_t i=0; i<items;i++){
        RedisModuleCallReply *ele = RedisModule_CallReplyArrayElement(reply,i);
        size_t l = RedisModule_CallReplyLength(ele);
        const char *val= RedisModule_CallReplyStringPtr(ele,&l);
        RedisModuleString *str = RedisModule_CreateString(ctx,val,l);
        
        RedisModuleCallReply *valReply = RedisModule_Call(ctx,"LRANGE","ccc",str ,"0","-1");
       
        
        size_t numItems = RedisModule_CallReplyLength(valReply);
         RedisModule_Log(ctx,"notice", "finished");
//        RedisModule_Log(ctx,"notice",numItems);
        
        for(size_t j=0;j<numItems;j++){
            
            RedisModuleCallReply *element = RedisModule_CallReplyArrayElement(valReply,j);
            size_t eleLen = RedisModule_CallReplyLength(element);
            const char *value = RedisModule_CallReplyStringPtr(element,&eleLen);
            
            
            
//            char arr[100];
//            strcpy(arr,val);
//            strcat(arr,value);
            
//             RedisModule_ReplyWithStringBuffer(ctx,arr,(RedisModule_CallReplyLength(element)));
        }
        
        
        
        
        RedisModule_ReplyWithStringBuffer(ctx,val,(RedisModule_CallReplyLength(ele)));
        
//        RedisModule_ReplyWithStringBuffer(ctx,value,(RedisModule_CallReplyLength(element)));
        
        replylen++;
        
    }
    
//
//
//
//
//    size_t len = RedisModule_CallReplyLength(reply);
//    const char *key = RedisModule_CallReplyStringPtr(reply, &len);
    
  //  RedisModule_ReplyWithCallReply(ctx,reply);
    
//    RedisModule_ReplyWithSimpleString(ctx,"finished");
//     RedisModule_ReplySetArrayLength(ctx,replylen);
    RedisModule_ReplySetArrayLength(ctx,replylen);
    
      return REDISMODULE_OK;
    
    
}
    



int RedisModule_OnLoad(RedisModuleCtx *ctx , RedisModuleString **argv, int argc){
    if(RedisModule_Init(ctx , "list_extend" , 1 , REDISMODULE_APIVER_1) == REDISMODULE_ERR){
        return REDISMODULE_ERR; }
    
    if(RedisModule_CreateCommand(ctx, "list_extend.filter" , ListExtendFilter_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR) { return REDISMODULE_ERR;}
    
    if(RedisModule_CreateCommand(ctx, "list_extend.sort",ListExtendSort_RedisCommand,"write deny-oom", 1, 1, 1)== REDISMODULE_ERR) { return REDISMODULE_ERR; }
    
    if(RedisModule_CreateCommand(ctx, "list_extend.splitString",ListExtendSplitString_RedisCommand,"write deny-oom", 1, 1, 1)== REDISMODULE_ERR) { return REDISMODULE_ERR; }
    
    if(RedisModule_CreateCommand(ctx, "list_extend.lexiSortStrings",ListExtendLexiSortString_RedisCommand,"write deny-oom", 1, 1, 1)== REDISMODULE_ERR) { return REDISMODULE_ERR; }
    
    if(RedisModule_CreateCommand(ctx, "list_extend.keys",Keys_RedisCommand,"write deny-oom", 1, 1, 1)== REDISMODULE_ERR) { return REDISMODULE_ERR; }
    
    if(RedisModule_CreateCommand(ctx, "list_extend.insert",ListInsert_RedisCommand,"write deny-oom", 1, 1, 1)== REDISMODULE_ERR) { return REDISMODULE_ERR; }
    
    if(RedisModule_CreateCommand(ctx, "list_extend.fetchkeyVal",ListFetchKeyVal_RedisCommand,"readonly getkeys-api", 1, 1, 1)== REDISMODULE_ERR) { return REDISMODULE_ERR; }
    
    if(RedisModule_CreateCommand(ctx, "list_extend.keyRange",listOfKeys_RedisCommand,"readonly", 1, 1, 0)== REDISMODULE_ERR) { return REDISMODULE_ERR; }
    
    if(RedisModule_CreateCommand(ctx, "list_extend.getKeys",getKeys_RedisCommand,"write deny-oom", 1, 1, 1)== REDISMODULE_ERR) { return REDISMODULE_ERR; }
    
    
    Keyspace = RedisModule_CreateDict(NULL);
    
    return REDISMODULE_OK;
    
}







