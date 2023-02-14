#include "QxLogger.h"

#include <cstdio>
#include <functional>
#include <vcruntime_typeinfo.h>


void QxLogger::Print(int e)
{
    printf ( " %04d", e );
}

void QxLogger::Print(float e)
{
    printf ( " %4.3f", e );
}

void QxLogger::Print(double e)
{
    printf ( " %4.3f", e );
}

void QxLogger::Print(char e)
{
    printf ( " %c", ( 31 < e ) && ( e < 128 ) ? e : '$' );
}
