#include <iostream>
#include <Windows.h>
#include <chrono>
using namespace std;

//global variable
int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4;
float fDepth = 16.0f;

int main() {
	//Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//Creating the map
	wstring map;
	map += L"################";
	map += L"#     #        #";
	map += L"#     #        #";
	map += L"#     #        #";
	map += L"#     #        #";
	map += L"#     #  #######";
	map += L"#              #";
	map += L"#              #";
	map += L"#              #";
	map += L"#              #";
	map += L"#              #";
	map += L"#              #";
	map += L"#     ##########";
	map += L"#              #";
	map += L"#              #";
	map += L"################";

	//time points 1 and 2
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	//Game loop
	while (true) {
		tp2 = chrono::system_clock::now(); //obtain current system time for each gameloop
		chrono::duration<float> elapsedTime = tp2 - tp1; //difference between current system time and previous system time
		tp1 = tp2; //update old time point to current time
		float fElapsedTime = elapsedTime.count(); //obtain elapsedtime as a floating point

		//Controls
		if (GetAsyncKeyState((unsigned short)'J') & 0x8000)
			fPlayerA -= (1.0f) * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'L') & 0x8000)
			fPlayerA += (1.0f) * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			fPlayerX -= cosf(fPlayerA) * 3.0f * fElapsedTime;
			fPlayerY += sinf(fPlayerA) * 3.0f * fElapsedTime;

			//Collision detection, correlating player position fo cells on the map
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += cosf(fPlayerA) * 3.0f * fElapsedTime; //"moving backwards"
				fPlayerY -= sinf(fPlayerA) * 3.0f * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			fPlayerX += cosf(fPlayerA) * 3.0f * fElapsedTime;
			fPlayerY -= sinf(fPlayerA) * 3.0f * fElapsedTime;

			//Collision detection, correlating player position fo cells on the map
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX -= cosf(fPlayerA) * 3.0f * fElapsedTime; //"moving backwards"
				fPlayerY += sinf(fPlayerA) * 3.0f * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * 3.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 3.0f * fElapsedTime;

			//Collision detection, correlating player position fo cells on the map
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX -= sinf(fPlayerA) * 3.0f * fElapsedTime; //"moving backwards"
				fPlayerY -= cosf(fPlayerA) * 3.0f * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * 3.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 3.0f * fElapsedTime;

			//Collision detection, correlating player position fo cells on the map
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += sinf(fPlayerA) * 3.0f * fElapsedTime; //"moving forward"
				fPlayerY += cosf(fPlayerA) * 3.0f * fElapsedTime;
			}
		}

		for (int x = 0; x < nScreenWidth; x++) {
			//Projecting a ray for each column to see how far the ray can travel before hitting the wall
			//(fPlayerA - fFOV / 2.0f) finds the starting angle of the ray, the second part determines which column the ray is on
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			//Decomposing RanAngle to x and y components
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			float fDistanceToWall = 0;
			bool bHitWall = false;
			bool bBoundary = false;

			while (!bHitWall && fDistanceToWall < fDepth) {
				fDistanceToWall += 0.1f;

				//Vector ray that attempts to find the wall
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//Testing if the ray has gone out of bound, note the map is in the first quadrant
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else {
					//Ray is inbound, converting TestX and TestY into the corresponding block on the map
					if (map[nTestY * nMapWidth + nTestX] == '#') {
						bHitWall = true;
					}
				}
			}

			//Calculate distance to ceiling and floor
			//Larger DistanceToWall corresponds to larger Ceiling value
			//Floor is the mirror of the ceiling (screenheight = 40, ceiling = 12 from top, floor = 28 from top or 12 form bottom)
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';
			short nShade_floor = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)		nShade = 0x2588;	//Solid block for very close wall
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;
			else										nShade = ' ';		//Blank space for blocks too far away

			//Although we are iterating from y = 0, (0, 0) in in fact the top left corner of the screen so we are going from top to bottom
			for (int y = 0; y < nScreenHeight; y++) {
				if (y < nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else {
					//Shading floor based on distance
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade_floor = '#';
					else if (b < 0.5)	nShade_floor = 'x';
					else if (b < 0.76)	nShade_floor = '.';
					else if (b < 0.9)	nShade_floor = '-';
					else				nShade_floor = ' ';
					screen[y * nScreenWidth + x] = nShade_floor;
				}
			}
		}
		//Display stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);
		
		//Display map
		for (int nx = 0; nx < nMapWidth; nx++) {
			for (int ny = 0; ny < nMapWidth; ny++) {
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		}
		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P'; //player position

		//Writing to the screen
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}