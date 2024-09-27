#include <iostream>
using namespace std;

bool loadBMP_custom(const char* imagepath, int& width, int& height, unsigned char* &data) {

	cout << "Reading image " << imagepath << "\n";

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	// Actual RGB data

	// Open the file
	FILE* file = fopen(imagepath, "rb");
	if (!file) {
		cout << imagepath << "could not be opened.Are you in the right directory ?";
		return false;
	}

	// Read the header, i.e. the 54 first bytes
	// If less than 54 bytes are read, problem
	if (fread(header, 1, 54, file) != 54) {
		cout <<"Not a correct BMP file"<<endl;
		fclose(file);
		return false;
	}
	// A BMP files always begins with "BM"
	if (header[0] != 'B' || header[1] != 'M') {
		cout<< "Not a correct BMP file"<<endl;
		fclose(file);
		return false;
	}
	// Make sure this is a 24bpp file
	if (*(int*)&(header[0x1E]) != 0) {cout << "Not a correct BMP file" << endl;  fclose(file); return 0; }
	if (*(int*)&(header[0x1C]) != 24) {cout << "Not a correct BMP file" << endl;    fclose(file); return 0; }

	// Read the information about the image
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width * height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fseek(file, dataPos, SEEK_SET);
	fread(data, 1, imageSize, file);

	// Everything is in memory now, the file can be closed.
	fclose(file);
	return true;
}