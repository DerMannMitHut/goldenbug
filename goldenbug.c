// MAKE: wall | make
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef unsigned int bugdirtype;

bugdirtype* bugdirs;
int size;
int arraylength;
int linelength;

int lastBugX;
int lastBugY;

int minX = 0;
int minY = 0;
int maxX = 0;
int maxY = 0;

// ============================== memory management

// calc the index of a given position
#define IDX_S_LL(X,Y,S,LL) ((X+S)+(Y+S)*LL)

inline int idx( int x, int y ){
    return IDX_S_LL(x,y,size,linelength);
}

// initialize with given size: allowed range for x and y is -size .. +size
void init( int newsize ){
    size = newsize;
    linelength = 2*size+1;
    arraylength = linelength*linelength;

    int bytes = arraylength*sizeof(bugdirtype);
    bugdirs = (bugdirtype*)malloc(bytes);
    memset(bugdirs, 0, bytes);
}

// initialize with doubled size (plus 1) and copy old data into new
void resize(){
    bugdirtype* oldbugdirs = bugdirs;
    int olds = size;
    int oldll = linelength;
    init( 2*size+1 );
    
    for( int x=-olds ; x<=olds ; ++x ){
	for( int y=-olds ; y<=olds ; ++y ){
	    bugdirs[idx(x,y)] = oldbugdirs[IDX_S_LL(x,y,olds,oldll)];
	}
    }

    free( oldbugdirs );
}

// is there a bug already at given position?
inline bugdirtype hasBug( int i ){
    return bugdirs[i] & 0x10;
}

// what is the current direction at given position?
inline bugdirtype getDir( int i ){
    return bugdirs[i] & 0x03;
}

// sets a bug to the given position
inline void setBug( int i ){
    bugdirs[i] |= 0x10;
}

// turn the direction by 90° and returns the new value
inline bugdirtype stepDir( int i ){
    bugdirtype oldbugdirs = bugdirs[i];
    bugdirtype newDir = (oldbugdirs+1) & 0x03;
    bugdirs[i] = (newDir) | (oldbugdirs & 0x10 );

    return newDir;
}

// ============================== algo

// the run of one bug (includes interpretation of directions)
void letTheNextBugRun(){
    int x = 0;
    int y = 0;
    int i = idx(x,y);

    while( hasBug(i) ){
	switch( stepDir(i) ){
	    case 0:
		x += 1;
		break;
	    case 1:
		y += 1;
		break;
	    case 2:
		x -= 1;
		break;
	    case 3:
		y -= 1;
		break;
	}

	i = idx(x,y);
    }

    setBug(i);

    lastBugX = x;
    lastBugY = y;
}

// output the current image in netpbm format P2 (see http://en.wikipedia.org/wiki/Netpbm_format)
void printDirections(){
    printf( "P2\n%d %d\n4\n", maxX-minX+1, maxY-minY+1 );
    for( int y=minY ; y<=maxY ; ++y ){
	for( int x=minX ; x<=maxX ; ++x ){
	    int i = idx(x,y);
	    if( hasBug(i) ){
		printf( "%d ", getDir(i) + 1 );
	    }
	    else {
		printf( "0 " );
	    }
	}
	printf( "\n" );
    }
    printf( "\n" );
}

// start given number of bugs (one after another)
//  * displays progress on per-mille-basis
//  * resizes field if bugs reach the edge of the field
void startRuns( int maxNumberOfBugs ){
    int numberOfBugs = 0;

    int stepTick = maxNumberOfBugs/1000;
    int nextTick = 0;

    while( numberOfBugs < maxNumberOfBugs ){
	letTheNextBugRun();
	numberOfBugs += 1;

	if( numberOfBugs >= nextTick ){
	    fprintf( stderr, "%d/%d\n", numberOfBugs, maxNumberOfBugs );
	    nextTick += stepTick;
	}

	if( lastBugX < minX ){
	    minX = lastBugX;
	    if( minX <= -size ){
		resize();
	    }
	}
	else if( lastBugX > maxX ){
	    maxX = lastBugX;
	    if( maxX >= size ){
		resize();
	    }
	}

	if( lastBugY < minY ){
	    minY = lastBugY;
	    if( minY <= -size ){
		resize();
	    }
	}
	else if( lastBugY > maxY ){
	    maxY = lastBugY;
	    if( maxY >= size ){
		resize();
	    }
	}
    }
}

// ============================== UNIT TESTS

int fail(char* errormsg, int retval){
    fprintf( stderr, "<%i> fail: %s\n", retval, errormsg );
    return retval;
}

int testInit(){
    fprintf( stderr, "testInit()..." );

    init(5);

    if( 5 != size ){
	return fail("size", 10010000);
    }

    for( int x=-5 ; x <= 5 ; ++x ){
	for( int y=-5 ; y <= 5 ; ++y ){
	    if( hasBug( idx(x,y) ) ){
		return fail("bugs", 10040000 + (x+5)+(y+5)*11);
	    }
	    if( 0 != getDir(idx(x,y)) ){
		return fail("dirs", 10050000 + (x+5)+(y+5)*11);
	    }
	}
    }

    
    fprintf( stderr, "ok.\n" );
    return 0;
}

int testInitAndResize(){
    fprintf( stderr, "testInitAndResize()..." );

    init(5);

    setBug( idx(-5,-5) );
    stepDir( idx(0,0) );

    resize();

    if( 11 != size ){
	return fail("size", 20010000);
    }

    if( !hasBug( idx(-5,-5) ) ){
	return fail( "copy bugs problem", 20020000 );
    }

    if( 1 != getDir( idx(0,0) ) ){
	return fail( "copy dirs problem", 20030000 );
    }


    for( int x=-11 ; x <= 11 ; ++x ){
	for( int y=-11 ; y <= 11 ; ++y ){
	    if( -5 != x && -5 != y ){
		if( hasBug( idx(x,y) ) ){
		    return fail("bugs", 20040000 + (x+11)+(y+11)*23);
		}
	    }

	    if( 0 != x && 0 != y ){
		if( 0 != getDir( idx(x,y) ) ){
		    return fail("dirs", 20050000 + (x+11)+(y+11)*23);
		}
	    }
	}
    }
    
    fprintf( stderr, "ok.\n" );
    return 0;
}

int testAccess(){
    fprintf( stderr, "testAccess()..." );

    init(5);

    setBug( idx(1,2) );
    if( !hasBug( idx(1,2) ) ){
	return fail( "hasBug", 30010000 );
    }

    stepDir( idx(1,2) );
    if( !hasBug( idx(1,2) ) ){
	return fail( "hasBug", 30020000 );
    }
    if( 1 != getDir( idx(1,2) ) ){
	return fail( "getDir", 30030000 );
    }


    stepDir( idx(3,4) );
    if( 1 != getDir( idx(3,4) ) ){
	return fail( "getDir", 30040000 );
    }

    fprintf( stderr, "ok\n" );
    return 0;
}

int testNextDir() {
    fprintf( stderr, "testNextDir()..." );

    init(5);

    for( int i=0 ; i<1000 ; ++i ){
	if( 1 != stepDir( idx(0,0) ) ){
	    return fail( "expected 1", 40010000+10*i+getDir( idx(0,0) ) );
	}

	if( 2 != stepDir( idx(0,0) ) ){
	    return fail( "expected 2", 40020000+10*i+getDir( idx(0,0) ) );
	}

	if( 3 != stepDir( idx(0,0) ) ){
	    return fail( "expected 3", 40030000+10*i+getDir( idx(0,0) ) );
	}

	if( 0 != stepDir( idx(0,0) ) ){
	    return fail( "expected 0", 40040000+10*i+getDir( idx(0,0) ) );
	}
    }

    fprintf( stderr, "ok\n" );
    return 0;
}



int test(){
    return //
	testInit() +
	testAccess() + 
	testInitAndResize() +
	testNextDir();
}


// ============================== main

// usage: ./goldenbug [<number of bugs>]
//
// if <number of bugs> is less than 1 (or not given at all): start the unit tests
// if <number of bugs> is 1 or greater: start the given number of bugs
int main( int argc, char** argv ){
    int maxNumberOfBugs = 0;
    if( argc>1 ){
	maxNumberOfBugs = atoi( argv[1] );
    }

    if( maxNumberOfBugs<1 ){
	int retval = test();
	if( 0 == retval ){
	    fprintf( stderr, "all tests ok.\n");
	}
	return retval;
    }
    else{
	init(10);
	startRuns( maxNumberOfBugs );
	printDirections();
	return 0;
    }
}
