// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/*============================================================
**
** Source: criticalsectionfunctions/test1/initializecriticalsection.c
**
** Purpose: Test Semaphore operation using classic IPC problem:
**          "Producer-Consumer Problem".
**
** Dependencies: CreateThread
**               InitializeCriticalSection
**               EnterCriticalSection
**               LeaveCriticalSection
**               DeleteCriticalSection
**               WaitForSingleObject
**               Sleep 
** 

**
**=========================================================*/

#include <palsuite.h>

#define PRODUCTION_TOTAL 26

#define _BUF_SIZE 10

DWORD dwThreadId;  /* consumer thread identifier */

HANDLE hThread; /* handle to consumer thread */

CRITICAL_SECTION CriticalSectionM; /* Critical Section Object (used as mutex) */

typedef struct Buffer
{
    short readIndex;
    short writeIndex;
    CHAR message[_BUF_SIZE];

} BufferStructure;

CHAR producerItems[PRODUCTION_TOTAL + 1];

CHAR consumerItems[PRODUCTION_TOTAL + 1];

/* 
 * Read next message from the Buffer into provided pointer.
 * Returns:  0 on failure, 1 on success. 
 */
int
readBuf(BufferStructure *Buffer, char *c)
{
    if( Buffer -> writeIndex == Buffer -> readIndex )
    {
	return 0;    
    }
    *c = Buffer -> message[Buffer -> readIndex++];
    Buffer -> readIndex %= _BUF_SIZE;
    return 1;
}

/* 
 * Write message generated by the producer to Buffer. 
 * Returns:  0 on failure, 1 on success.
 */
int 
writeBuf(BufferStructure *Buffer, CHAR c)
{
    if( ( ((Buffer -> writeIndex) + 1) % _BUF_SIZE) ==
	(Buffer -> readIndex) )
    {
	return 0;
    }
    Buffer -> message[Buffer -> writeIndex++] = c;
    Buffer -> writeIndex %= _BUF_SIZE;
    return 1;
}

/* 
 * Sleep 500 milleseconds.
 */
VOID 
consumerSleep(VOID)
{
    Sleep(500);
}

/* 
 * Sleep between 10 milleseconds.
 */
VOID 
producerSleep(VOID)
{
    Sleep(10);
}

/* 
 * Produce a message and write the message to Buffer.
 */
VOID
producer(BufferStructure *Buffer)
{

    int n = 0;
    char c;
    
    while (n < PRODUCTION_TOTAL) 
    {
	c = 'A' + n ;   /* Produce Item */   
	
	EnterCriticalSection(&CriticalSectionM);
	
	if (writeBuf(Buffer, c)) 
	{
            printf("Producer produces %c.\n", c);
	    producerItems[n++] = c;
	}
	
	LeaveCriticalSection(&CriticalSectionM);
	
	producerSleep();
    }

    return;
}

/* 
 * Read and "Consume" the messages in Buffer. 
 */
DWORD
PALAPI 
consumer( LPVOID lpParam )
{
    int n = 0;
    char c; 
    
    consumerSleep();
    
    while (n < PRODUCTION_TOTAL) 
    {
	
	EnterCriticalSection(&CriticalSectionM);
	
	if (readBuf((BufferStructure*)lpParam, &c)) 
	{
	    printf("\tConsumer consumes %c.\n", c);
	    consumerItems[n++] = c;
	}
	
	LeaveCriticalSection(&CriticalSectionM);
	
	consumerSleep();
    }
    
    return 0;
}

int __cdecl main (int argc, char **argv) 
{

    BufferStructure Buffer, *pBuffer;
    
    pBuffer = &Buffer;
    
    if(0 != (PAL_Initialize(argc, argv)))
    {
	return FAIL;
    }
    
    /*
     * Create mutual exclusion mechanisms
     */
    
    InitializeCriticalSection ( &CriticalSectionM );
    
    /* 
     * Initialize Buffer
     */
    pBuffer->writeIndex = pBuffer->readIndex = 0;
    
    
    
    /* 
     * Create Consumer
     */
    hThread = CreateThread(
	NULL,         
	0,            
	consumer,     
	&Buffer,     
	0,           
	&dwThreadId);
    
    if ( NULL == hThread ) 
    {
	Fail ( "CreateThread() returned NULL.  Failing test.\n"
	       "GetLastError returned %d\n", GetLastError());   
    }
    
    /* 
     * Start producing 
     */
    producer(pBuffer);
    
    /*
     * Wait for consumer to complete
     */
    WaitForSingleObject (hThread, INFINITE);
    
    /* 
     * Compare items produced vs. items consumed
     */
    if ( 0 != strncmp (producerItems, consumerItems, PRODUCTION_TOTAL) )
    {
	Fail("The producerItems string %s\n and the consumerItems string "
	     "%s\ndo not match. This could be a problem with the strncmp()"
	     " function\n FailingTest\nGetLastError() returned %d\n", 
	     producerItems, consumerItems, GetLastError());
    }
    
    /* 
     * Clean up Critical Section object 
     */
    DeleteCriticalSection(&CriticalSectionM);
    
    Trace("producerItems and consumerItems arrays match.  All %d\nitems "
	  "were produced and consumed in order.\nTest passed.\n",
	  PRODUCTION_TOTAL);
    
    PAL_Terminate();
    return (PASS);

}
