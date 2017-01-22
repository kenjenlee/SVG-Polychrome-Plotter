#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream> 

using namespace std;



// Eric Chee
string parsePath(istream & fin, double scaleFactor,
                 double & xInit, double & yInit);
// Ken Lee
string parseCubic(double cx1, double  cy1, double  cx2,
                  double  cy2, double  xi,
                  double  yi, double  xf, double  yf, double scaleFactor);
// Ken Lee
string parseQuad(double cx, double  cy, double  xi, double  yi,
                 double  xf, double yf, double scaleFactor);
// Karan Bajaj
string parseMove(double  xf, double yf, double scaleFactor);
// Karan Bajaj
string parseRectangle(istream & fin, double scaleFactor, double & xInit,
                      double & yInit);
// Mulan Ramani
string parseLine(double  xf, double yf, double scaleFactor);
// Mulan Ramani
string parseLine(istream & fin, double scaleFactor, double & xInit,
                 double & yInit);


// Eric Chee
int main()
{

	//Measured values from the plotter
	const double PLOTTER_HEIGHT = 500;
	const double PLOTTER_WIDTH = 460;

	ifstream fin("beditry.svg");//modified svg file
	if (!fin)
	{
		cout << "Failed to open file";
		return -1;
	}


	string temp = "", commandSet = "", command = "1";
	double width = 0, height = 0, scaleFactor = 0;
	bool parseFile = 0;
	double x = 0, y = 0;
	int colour = 0;
	string colours[6] = { "" };//Command sets for each colour

							   //Finds start of svg file
	while (fin >> temp && !parseFile)
	{
		if (temp == "svg")
		{
			fin >> width >> height;
			scaleFactor = min((PLOTTER_HEIGHT / height),
                              (PLOTTER_WIDTH / width));
			parseFile = 1;
		}
	}
	//Parses svg command sets
	do //do loop because first command is read in when
       //finding the start
	{
		fin >> colour;
		if (temp == "path")
		{
			commandSet = parsePath(fin, scaleFactor, x, y);
		}
		else if (temp == "rect")
		{
			commandSet = parseRectangle(fin, scaleFactor, x, y);
		}
		else if (temp == "line")
		{
			commandSet = parseLine(fin, scaleFactor, x, y);
		}
		colours[colour - 1] += commandSet;
	} while (fin >> temp&&parseFile&&temp != "/svg>");

	//outputs commands to file
	ofstream fout("beditry.txt");
	for (int i = 0; i<6; i++)
	{
		if(!colours[i].empty())//if not empty
			fout << i + 1 << " " << colours[i] << " -1" << endl;
	}
	fin.close();
	fout.close();
	return 0;
}


string parsePath(istream & fin, double scaleFactor,
                 double & xInit, double & yInit)
{
	string junk, temp;
	double cx1, cy1, cx2, cy2, xChange, yChange,
	xStart = xInit, yStart = yInit;
	stringstream commandSet;
	char command = ' ', prevCommand = ' ';

	//Lower case is relative to previous
	//Upper case is absolute to (0, 0)

	//Parse sub commands
	while (command != '>')//'>' is the end of path sentry
	{
		fin >> command;

		//Cubic curves
		if (command == 'c' || command == 'C')
		{
			fin >> cx1 >> cy1 >> cx2 >> cy2 >> xChange >> yChange;
			if (command == 'C')
			{
				xChange -= xStart;
				yChange -= yStart;
				cx1-=xStart;
				cx2-=xStart;
				cy1-=yStart;
				cy2-=yStart;
			}
			commandSet << parseCubic(cx1, cy1, cx2, cy2,
                            xStart, yStart, xChange, yChange, scaleFactor);
		}

		//Shorthand cubic curves
		else if (command == 's' || command == 'S')
		{
			if (prevCommand == 'c' || prevCommand == 's' ||
                prevCommand == 'C' || prevCommand == 'S')
			{
				cx1 = xStart + (xStart - cx2);
				cy1 = xStart + (xStart - cy2);
			}
			else//if no previous control point use surrent position
                //as control point 1
			{
				cx1 = xStart;
				cy1 = yStart;
			}
			fin >> cx2 >> cy2 >> xChange >> yChange;
			if (command == 'S')
			{
				xChange -= xStart;
				yChange -= yStart;

			}
			commandSet << parseCubic(cx1, cy1, cx2, cy2, xStart, yStart,
                                     xChange, yChange, scaleFactor);
		}


		//Quadratic Curves
		else if (command == 'q' || command == 'Q')
		{
			fin >> cx1 >> cy1 >> xChange >> yChange;
			if (command == 'Q')
			{
				xChange -= xStart;
				yChange -= yStart;
				cx1-=xStart;
				cy1-=yStart;
			}
			commandSet << parseQuad(cx1, cy1, xStart, yStart,
                           xChange, yChange, scaleFactor);

		}

		//Shorthand quadratic curves
		else if (command == 't' || command == 'T')
		{
			if (prevCommand == 't' || prevCommand == 'q' ||
                prevCommand == 'T' || prevCommand == 'Q')
			{
				cx1 = xStart + (xStart - cx1);
				cy1 = yStart + (yStart - cy1);
			}
			else
			{
				cx1 = xStart;
				cy1 = yStart;
			}
			fin >> xChange >> yChange;
			commandSet << parseQuad(cx1, cy1, xStart, yStart,
                           xChange, yChange, scaleFactor);
			if (command == 'T')
			{
				xChange -= xStart;
				yChange -= yStart;
			}

		}

		//(Straight) Lines
		else if (command == 'l')
		{
			fin >> xChange >> yChange;
			commandSet << parseLine(xStart + xChange,
                           yStart + yChange, scaleFactor);
		}
		else if (command == 'L')
		{
			fin >> xChange >> yChange;
			commandSet << parseLine(xChange, yChange, scaleFactor);
		}

		//Move to
		else if (command == 'm')
		{
			fin >> xChange >> yChange;
			commandSet << parseMove(xStart + xChange,
                           yStart + yChange, scaleFactor);
		}
		else if (command == 'M')
		{
			fin >> xChange >> yChange;
			commandSet << parseMove(xChange, yChange, scaleFactor);

		}

		//Return to start
		else if (command == 'z' || command == 'Z')
		{
			xChange = xStart - xInit;
			yChange = yStart - yInit;
			commandSet << parseLine(xChange, yChange, scaleFactor);
		}

		prevCommand = command;

        // if commands were absolute
		if(command == 'C' || command=='M' || command=='Z'
           || command=='L' )
            {
                xStart = xChange;
                yStart = yChange;
            }
        else
            {
                xStart += xChange;
                yStart += yChange;
            }
	}

	//sets the current position of the head
	xInit = xStart;
	yInit = yStart;

	return commandSet.str();
}

//Note all coordinates are scaled at the point of conversion to
//plotter points to in avoid confusion of what scale point
//are in other methods

string parseRectangle(istream & fin, double scaleFactor,
                      double & xInit, double & yInit)
{
	double xi, yi, width, height;

	fin >> xi >> yi >> width >> height;
	xInit = round(xi);
	yInit = round(yi);
	xi *= scaleFactor;
	yi *= scaleFactor;
	width *= scaleFactor;
	height *= scaleFactor;

	stringstream commandStream;

	commandStream << " 0 " << round(xi) << " " << round(yi) << " "
		<< "1 " << round(xi + width) << " " << round(yi) << " "
		<< "1 " << round(xi + width) << " " << round(yi + height) << " "
		<< "1 " << round(xi) << " " << round(yi + height) << " "
		<< "1 " << round(xi) << " " << round(yi) << " ";
	return commandStream.str();
}

//Line command
string parseLine(istream & fin, double scaleFactor,
                 double & xInit, double & yInit)
{
	double xi, yi, xf, yf;

	fin >> xi >> yi >> xf >> yf;
	xInit = round(xf);
	yInit = round(yf);
	xi *= scaleFactor;
	yi *= scaleFactor;
	xf *= scaleFactor;
	yf *= scaleFactor;

	stringstream commandStream;

	commandStream << " 0 " << round(xi) << " " << round(yi) << " "
		<< "1 " << round(xf) << " " << round(yf) << " ";

	return commandStream.str();
}

//Line subcommand of Path
string parseLine(double  xf, double yf, double scaleFactor)
{
	xf *= scaleFactor;
	yf *= scaleFactor;
	stringstream commandStream;

	commandStream << "1 " << round(xf) << " " << round(yf) << " ";
	return commandStream.str();
}

string parseMove(double  xf, double yf, double scaleFactor)
{
	xf *= scaleFactor;
	yf *= scaleFactor;
	stringstream commandStream;

	commandStream << "0 " << round(xf) << " " << round(yf) << " ";

	return commandStream.str();
}

string parseQuad(double cx, double cy, double xi, double yi,
                 double xf, double yf, double scaleFactor)
{
    //Make relative points absolute
    cx = (xi+cx)*scaleFactor;
    cy = (yi+cy)*scaleFactor;
    xf = (xi + xf)*scaleFactor;
    yf = (yi + yf)*scaleFactor;
    xi *= scaleFactor;
    yi *= scaleFactor;

    // Define necessary parameters
    double xLast = xi,
		   xCurrent = 0.0,
           yLast = yi,
           yCurrent = 0.0;

   
    double a1x = xi-2*cx+xf,
           a2x = 2*cx-2*xi,
           a1y = yi-2*cy+yf,
           a2y = 2*cy-2*yi,
           t = 0.00;

    stringstream cood;


    // t=0 at start of curve and t=1 at end of curve
    for(;t<1.00; t+=0.01)
    {

    	// Delta x and y set to a minimum of 2 units
    	do
		{
		    t+=0.01;
    		xCurrent = a1x*t*t + a2x*t + xi;
    		yCurrent = a1y*t*t + a2y*t + yi;
		}while((abs(xCurrent-xLast)<2.0 ||
               abs(yCurrent - yLast)<2.0) && t<1.0);

		//Extra checking for invalid points and recalculations
		// until valid points ae generated
		while((round(yCurrent)<0 || round(xCurrent)<0 ||
               round(yCurrent)>500 || round(xCurrent)>460)&&t<1.0 )
        {
                t+=0.01;
                xCurrent = a1x*t*t + a2x*t + xi;
                yCurrent = a1y*t*t + a2y*t + yi;
        }

        //If no valid points can be found, coordinates
        // assume the previous set of coordinates
        if(round(yCurrent)<0 || round(xCurrent)<0 ||
           round(yCurrent)>500 || round(xCurrent)>460)
        {
            yCurrent = yLast;
            xCurrent = xLast;

        }
        // include new coordinates into string
		cood << " 1 " << round(xCurrent) << " " << round(yCurrent);
		//Update previous set of coordinates
		yLast = yCurrent;
		xLast = xCurrent;
	}

	// Draws line to final coordinate if applicable
    if(xCurrent != xf || yLast != yf)
    {
        cood << " 1 " << round(xf) << " " << round(yf);
    }
    // outputs the string of coordinates
    return cood.str();
}

string parseCubic (double cx1, double cy1, double cx2, double cy2,
                   double xi, double yi, double xf, double yf,
                   double scaleFactor)
{

    //Make relative points absolute.
    cx1 = (xi+cx1)*scaleFactor;
    cy1 = (xi+cy1)*scaleFactor;
    cx2 = (xi+cx2)*scaleFactor;
    cy2 = (yi+cy2)*scaleFactor;
    xf = (xi + xf)*scaleFactor;
    yf = (yi + yf)*scaleFactor;
    xi *=scaleFactor;
    yi *=scaleFactor;

    //Define necessary variables for loop
    double xCurrent = 0.0,
		   xLast = xi,
           yLast = yi,
           yCurrent = 0.0;

    
    double a3x = 3*(cx1-xi),
           a2x = 3*(cx2-cx1)-a3x,
           a1x = xf-xi-a3x-a2x,
           a3y = 3*(cy1-yi),
           a2y = 3*(cy2-cy1)-a3y,
           a1y = yf-yi-a3y-a2y,
		   t = 0.0;

    //Used to store the string
    stringstream cood;

    // t has a range of [0,1] from the start to end of curve
    for(;t<1.0;t+=0.01)
    {
    	// Minimum delta x and y set to a minimum of 2 units
    	do
		{
    		t+=0.01;
    		xCurrent = a1x*t*t*t + a2x*t*t + a3x*t + xi;
    		yCurrent = a1y*t*t*t + a2y*t*t + a3y*t + yi;
		}while((abs(xCurrent-xLast)<2.0 ||
               abs(yCurrent-yLast)<2.0)&&t<1.0);

		//Extra checking for invalid points and recalculations
		// until valid points ae generated
		while((round(yCurrent)<0 || round(xCurrent)<0 ||
              round(yCurrent)>500 || round(xCurrent)>460)&&t<1.0 )
        {
                t+=0.01;
                xCurrent = a1x*t*t*t + a2x*t*t + a3x*t + xi;
                yCurrent = a1y*t*t*t + a2y*t*t + a3y*t + yi;
        }

        //If no valid points can be found, coordinates assume
        // the previous set of coordinates
        if(round(yCurrent)<0 || round(xCurrent)<0 ||
           round(yCurrent)>500 || round(xCurrent)>460)
        {
            yCurrent = yLast;
            xCurrent = xLast;

        }

        // include new coordinates into string
    	cood<< " 1 " << round(xCurrent) << " " << round(yCurrent);

    	//Update previous set of coordinates
    	yLast = yCurrent;
    	xLast = xCurrent;
	}

    // Draws line to final coordinate if applicable
    if(xCurrent != xf || yLast != yf)
    {
        cood << " 1 " << round(xf) << " " << round(yf);
    }

    // outputs the string of coordinates
    return cood.str();
}
