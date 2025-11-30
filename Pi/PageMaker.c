#include <stdio.h>
#include "LiDAR.h"				//	"global" stuff for the project as a whole
#include "mem.c"				//	mem functions for project
#include "PageMaker.h"

int main(int argc, char *argv[] )
{
	Rmem_ptr		mem_ptr = 0;
	int				shmid = 0;

	FILE			*html;
	
	//	get shared memory
	shmid = getShmid( configFile, PROJ_ID, sizeof(Rmem_typ) );
	
	/* Attach (map) the shared memory segment into the current process */
	if((mem_ptr = (Rmem_ptr)shmat(shmid, 0, 0)) == (Rmem_ptr)-1)
	{
		perror("shmat");
		exit(1);
	}

	setMkPgFlag( mem_ptr );	//	for testing
	
	// run until we get a kill message
	while( notDone(mem_ptr) )
	{
		//  two things need to happen before we can move the motor
			//	1) deleted - I was wrong...  WRONG, I say!
			//	2) we need indication that the data process is finished reading data
		if( readMkPgFlag(mem_ptr) )
		{
			clearMkPgFlag( mem_ptr );					//	Clear the flag
			
			html = fopen( tempFileName, "w+" );	//	open output file (defined in LiDAR.h)
			
			PrintStart( html );					//	print the beginning partsof the html file
			PrintAzInd( html, mem_ptr );		//	print the section for the azimuth indication
			PrintOldData( html, mem_ptr );
			PrintNewData( html, mem_ptr );

			PrintEnd( html );					//	print the end of the html file
			
			fclose( html );								//	close the output file
			
			rename( tempFileName, htmlFileName );
		}
		
		
//		if( _kbhit() )		//	Got a keyboard input - kill everything
//			setDoneFlag( mem_ptr );
	}
	
	//	okay - we got the DONE flag, flag memory for deletion
	if( shmctl(shmid, IPC_RMID, 0) == -1)
	{
		printf("big fucking error on shmctl!\n");
		return -1;
	}
	
	printf("Shared memory segment marked for deletion\n");

	//	and the detach from it
//	shmdt();

	return 0;
}

void PrintStart( FILE *html )
{
	int	range;
	
	fprintf(html, "<!DOCTYPE html>\n<html>\n<head>\n<title>SONAR Plot</title>\n");
	fprintf(html, "<meta http-equiv=\"refresh\" content=\"1\">\n</head>\n<body>\n");
	fprintf(html, "<h1>\n<center>\nRADAR PLOT\n</center>\n</h1>\n<hr>\n");
	fprintf(html, "<center>\n<canvas id=\"myCanvas\" width=\"1400\" height=\"750\"\n");
	fprintf(html, "style=\"border:1px solid #c3c3c3;\">\n");
	fprintf(html, "Your browser does not support the canvas element.\n</canvas>\n</center>\n");
	fprintf(html, "<script>\nvar canvas = document.getElementById(\"myCanvas\");\n");
	fprintf(html, "var ctx = canvas.getContext(\"2d\");\nctx.font=\"20px Arial\";\n");
	fprintf(html, "ctx.strokeStyle=\"#000000\";\n");

	//	CSS for the baseline
	fprintf(html, "%s\n", ctxBeginPath );
	fprintf(html, "ctx.moveTo(0,600);\nctx.lineTo(1400,600);\n");
	fprintf(html, "%s\n", ctxStroke );

	//	CSS for nadir line
	fprintf(html, "%s\n", ctxBeginPath );
	fprintf(html, "ctx.moveTo(700,600);\nctx.lineTo(700,500);\n");
	fprintf(html, "%s\n", ctxStroke );
	
	//	CSS for right sweep extent
	fprintf(html, "%s\n", ctxBeginPath );
	fprintf(html, "ctx.moveTo(700,600);\nctx.lineTo(1306,250);\n");
	fprintf(html, "%s\n", ctxStroke );

	//	CSS for left sweep extent
	fprintf(html, "%s\n", ctxBeginPath );
	fprintf(html, "ctx.moveTo(700,600);\nctx.lineTo(94,250);\n");
	fprintf(html, "%s\n", ctxStroke );

	//	CSS for range rings
	fprintf(html, "%s\n", ctxBeginPath );
	fprintf(html, "ctx.arc(700,600,1,0,2*Math.PI);\n");
	fprintf(html, "%s\n", ctxStroke );

	for( range=100 ; range<900 ; range=range+100 )
	{
		fprintf(html, "%s\n", ctxBeginPath );
		fprintf(html, "ctx.arc(700,600,%d,0,2*Math.PI);\n", range);
		fprintf(html, "%s\n", ctxStroke );
	}

	fprintf(html, "ctx.fillText(\"1m\",800,600);\n");
	fprintf(html, "ctx.fillText(\"2m\",900,600);\n");
	fprintf(html, "ctx.fillText(\"3m\",1000,600);\n");
	fprintf(html, "ctx.fillText(\"4m\",1100,600);\n");
	fprintf(html, "ctx.fillText(\"5m\",1200,600);\n");
	fprintf(html, "ctx.fillText(\"6m\",1300,600);\n");
	fprintf(html, "ctx.fillText(\"7m\",1350,300);\n");
	fprintf(html, "ctx.fillText(\"8m\",1350,100);\n");


	return;
}

void PrintEnd( FILE *html )
{
	fprintf( html, "</script>\n</body>\n</html>\n" );

	return;
}

void PrintAzInd( FILE *html, Rmem_ptr mem_ptr )
{
	fprintf(html, "ctx.strokeStyle=\"#FF0000\";\n" );
	fprintf(html, "%s\n", ctxBeginPath );
	fprintf(html, "ctx.moveTo(700,600);\n" );
	fprintf(html, "ctx.lineTo(%d,%d);\n", mem_ptr->currentPass[SENSOR_POS].X, \
											 mem_ptr->currentPass[SENSOR_POS].Y);
	fprintf(html, "%s\n", ctxStroke );
	
	return;
}

void PrintOldData( FILE *html, Rmem_ptr mem_ptr )
{
	fprintf(html, "ctx.strokeStyle=\"#00FF00\";\n" );

	for( int angle=MIN_ANGLE ; angle<=MAX_ANGLE ; angle++ )
	{
		fprintf(html, "%s\n", ctxBeginPath );
		fprintf(html, "ctx.arc(%d,%d,%d,0,2*Math.PI);\n", mem_ptr->lastPass[angle].X, \
															mem_ptr->lastPass[angle].Y, \
															mem_ptr->lastPass[angle].size );
		fprintf(html, "%s\n", ctxStroke );		
	}
	return;
}

void PrintNewData( FILE *html, Rmem_ptr mem_ptr )	//	print the section for the new range data
{
	fprintf(html, "ctx.strokeStyle=\"#0000FF\";\n" );

	for( int angle=MIN_ANGLE ; angle<=MAX_ANGLE ; angle++ )
	{
		fprintf(html, "%s\n", ctxBeginPath );
		fprintf(html, "ctx.arc(%d,%d,%d,0,2*Math.PI);\n", mem_ptr->currentPass[angle].X, \
															mem_ptr->currentPass[angle].Y, \
															mem_ptr->currentPass[angle].size );
		fprintf(html, "%s\n", ctxStroke );		
	}
	return;
}
