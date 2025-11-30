#ifndef __PageMaker_H__
#define __PageMaker_H__

const char ctxBeginPath[] = "ctx.beginPath();";
const char ctxStroke[] = "ctx.stroke();";

void PrintStart( FILE *html );						//	prints the beginning of the html file
void PrintEnd( FILE *html );						//	prints the end of the html file
void PrintAzInd( FILE *html, Rmem_ptr mem_ptr );	//	print the section for the azimuth indication
void PrintOldData( FILE *html, Rmem_ptr mem_ptr );	//	print the section for the old range data
void PrintNewData( FILE *html, Rmem_ptr mem_ptr );	//	print the section for the new range data

#endif
