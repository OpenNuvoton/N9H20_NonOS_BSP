/*
 *  tslib/src/ts_getxy.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: testutils.c,v 1.2 2004/10/19 22:01:27 dlowder Exp $
 *
 * Waits for the screen to be touched, averages x and y sample
 * coordinates until the end of contact
 */

#include "N9H20.h"
#include "tslib.h"
#include "fbutils.h"
#include "N9H20TouchPanel.h"

static int sort_by_x(const void* a, const void *b)
{
    return (((struct ts_sample *)a)->x - ((struct ts_sample *)b)->x);
}

static int sort_by_y(const void* a, const void *b)
{
    return (((struct ts_sample *)a)->y - ((struct ts_sample *)b)->y);
}

void getxy(int *x, int *y)
{
#define MAX_SAMPLES 128
    struct ts_sample samp[MAX_SAMPLES];
    int index, middle;
    int sumx, sumy;

    sysprintf("getxy\n");
again:
    do
    {
        if ( Read_TouchPanel(&sumx, &sumy) > 0 )
        {
            if ( (sumx < 0) || ( sumy < 0 ) )
                continue;
            break;
        }
    }
    while (1);

    /* Now collect up to MAX_SAMPLES touches into the samp array. */
    index = 0;
    do
    {
        if (index < MAX_SAMPLES-1)
            index++;
        if ( Read_TouchPanel(&sumx, &sumy) > 0)
        {
            samp[index].x = sumx;
            samp[index].y = sumy;
            samp[index].pressure = 1000;
        }
        else
        {
            samp[index].x = samp[index-1].x;
            samp[index].y = samp[index-1].y;
            samp[index].pressure = 0;
        }

//      sysprintf("%d %d %d\n", samp[index].x, samp[index].y , samp[index].pressure);
    }
    while (samp[index].pressure > 0);
    sysprintf("Took %d samples...\n",index);

    /*
     * At this point, we have samples in indices zero to (index-1)
     * which means that we have (index) number of samples.  We want
     * to calculate the median of the samples so that wild outliers
     * don't skew the result.  First off, let's assume that arrays
     * are one-based instead of zero-based.  If this were the case
     * and index was odd, we would need sample number ((index+1)/2)
     * of a sorted array; if index was even, we would need the
     * average of sample number (index/2) and sample number
     * ((index/2)+1).  To turn this into something useful for the
     * real world, we just need to subtract one off of the sample
     * numbers.  So for when index is odd, we need sample number
     * (((index+1)/2)-1).  Due to integer division truncation, we
     * can simplify this to just (index/2).  When index is even, we
     * need the average of sample number ((index/2)-1) and sample
     * number (index/2).  Calculate (index/2) now and we'll handle
     * the even odd stuff after we sort.
     */
    middle = index/2;
    if (x)
    {
        qsort(samp, index, sizeof(struct ts_sample), sort_by_x);
        if (index & 1)
            *x = samp[middle].x;
        else
            *x = (samp[middle-1].x + samp[middle].x) / 2;
    }
    if (y)
    {
        qsort(samp, index, sizeof(struct ts_sample), sort_by_y);
        if (index & 1)
            *y = samp[middle].y;
        else
            *y = (samp[middle-1].y + samp[middle].y) / 2;
    }
    if ( (index <= 3) || ( *x < 0) || ( *y < 0 ) )
        goto again;
}
