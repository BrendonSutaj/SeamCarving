// Standard Stuff.
#include <stdlib.h>
#include <stdio.h>

// For the functions isdigit etc.
#include <ctype.h>

// Argumentline Options.
#include <getopt.h>

// For char* operations like strrchr.
#include <string.h>

// For file opening error handling.
#include <unistd.h>

// For INT_MAX and INT_MIN.
#include <limits.h>

// Function Declarations.
int* checkFormat(FILE* file, char* filename);
int colorDifference(int r_1, int g_1, int b_1, int r_2, int g_2, int b_2);
void computeStats(int* data);
int* computeMinPath(int* data, int width);
void removePixels(int* rowValues, int* data, int width);
void writeDataToOut(int* data);
int min(int x, int y);

// Global Variables.
int WIDTH = 0;
int HEIGHT = 0;


int main(int const argc, char** const argv)
{
    // Argument flags, indicating whether they were used or not. (Initial value is 0, which stands for "false")
    int c = 0;
    int s_flag = 0;
    int p_flag = 0;
    int n_flag = 0;
    int count = 0;

    // Using POSIX optarg, setting flags if the respective option is given in argv.
    // '?' means that some error occurred -> exit_fail.
    while ((c = getopt(argc, argv, "spn:")) != -1)
    {
        switch (c)
        {
            case 's':
                s_flag = 1;
                break;
            case 'p':
                p_flag = 1;
                break;
            case 'n':
                count = atoi(optarg);
                n_flag = 1;
                break;
            case '?':
                exit(EXIT_FAILURE);
        }
    }

    // The filename has to be at the optind position, since optarg reordered the arguments in argv.
    char* filename = optind < argc ? argv[optind] : NULL;
    FILE* file;

    // If the filename is non existent, exit_fail.
    if (filename == NULL)
        exit(EXIT_FAILURE);


    // Check if the file exists, if it does not -> exit_fail, open otherwise.
    if (access(filename, F_OK) != -1) {
        file = fopen(filename, "rb");
    } else {
        exit(EXIT_FAILURE);
    }

    // File could not be opened.
    if (file == NULL)
        exit(EXIT_FAILURE);

    
    // Check the format for errors and get the values of the pixels into data.
    int* data = checkFormat(file, filename);

    // The s_flag is prioritized. Compute the stats, free the data and return.
    if (s_flag) {
        computeStats(data);
        free(data);
        exit(EXIT_SUCCESS);
    }

    // Next one in the priority list is the p_flag. Compute the minPath and print it to the console. Free data and rows and return.
    if (p_flag) {
        int* rows = computeMinPath(data, WIDTH);

        for (int i = 0; i < HEIGHT; i++) {
            printf("%d\n", rows[i]);
        }
        free(data);
        free(rows);
        exit(EXIT_SUCCESS);
    }

    // If we reach this code part, check if the n_flag is set.
    // If it is not set or count == - 1 we set it to WIDTH.
    count = (!n_flag || count == -1) ? WIDTH
            : count;

    // It has to be in the [-1, WIDTH] range.
    if (count > WIDTH || count < -1)
        exit(EXIT_FAILURE);
    
    // Shortcut if count == WIDTH
    if (count == WIDTH) {
        for (int x = 0; x < WIDTH * 3; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                data[x + y * WIDTH * 3] = 0;
            }
        }
        // Finally write the output image, free the data and return.
        writeDataToOut(data);
        free(data);
        exit(EXIT_SUCCESS);
    }

    // Iterate it count - times over, every time computing the minPath and then removing it from the data.
    // Free the rows for each iteration, to avoid heap buffer overflow.
    int width = WIDTH;
    while (count > 0) {
        int* rows = computeMinPath(data, width);
        removePixels(rows, data, width);
        free(rows);
        count--;
        width--;
    }

    // Finally write the output image, free the data and return.
    writeDataToOut(data);
    free(data);
    exit(EXIT_SUCCESS);
}

/**
 *
 * This function should check for errors in the file given, if there are no errors the values are returned.
 * @param file
 * @param filename
 * @return the data.
 */
int* checkFormat(FILE* file, char* filename) {
    int zero = 48;
    int newline = 10;
    int space = 32;

    // Check for the correct file ending first.
    const char *fileEnding = strrchr(filename, '.');
    if (!fileEnding || fileEnding == filename || strcmp("ppm", fileEnding + 1))
        exit(EXIT_FAILURE);
    

    // Go through the file to determine the fileSize in bytes.
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);

    // Reset to the start position of the file.
    fseek(file, 0, SEEK_SET);

    // Get all the chars as a string, stored into fileContent.
    // Allocating 1 more byte, for appropriately terminating the string with a 0 byte.
    unsigned char *fileContent = malloc(fileSize + 1);
    fread(fileContent, 1, fileSize, file);
    fileContent[fileSize] = 0;

    // Close the file finally.
    fclose(file);

    // In the first row, the ppm format needs to have a "P3\n".
    if (fileContent[0] != 'P' || fileContent[1] != '3' || fileContent[2] != newline)
        exit(EXIT_FAILURE);


    int width = 0;
    int height = 0;


    // The next one has to be the width. Store the width.
    if (!isdigit(fileContent[3]))
        exit(EXIT_FAILURE);
    
    int i = 3;
    while (isdigit(fileContent[i])) {
        width = width * 10 + fileContent[i++] - zero;
    }
    
    
    // At least one whitespace after width is needed.
    if (fileContent[i] != space)
        exit(EXIT_FAILURE);

    // Now skip all additional whitespaces.
    while (i <= fileSize && fileContent[i] == space) {
        i++;
    }

    // The next byte after the whitespaces needs to be the height. Store the height.
    if (!isdigit(fileContent[i]))
        exit(EXIT_FAILURE);

    while (isdigit(fileContent[i])) {
        height = height * 10 + fileContent[i++] - zero;
    }
    
    // Skip Whitespaces.
    while(i <= fileSize && fileContent[i] == space) {
        i++;
    }

    // Width and height needs to be > 0 and a newline has to be following.
    if (width <= 0 || height <= 0 || fileContent[i] != newline)
        exit(EXIT_FAILURE);
    
    
    // The next two bytes need to be 255 and \n.
    if (fileContent[++i] != 2 + zero || fileContent[++i] != 5 + zero || fileContent[++i] != 5 + zero || fileContent[++i] != newline)
        exit(EXIT_FAILURE);
    

    // Skip whitespaces, decrease count for every digit found. There have to be width * height * (red, green, blue) value many entries.
    int count = width * height * 3;
    for (int j = i + 1; j <= fileSize; j++) {
        
        if (fileContent[j] == 0)
            break;
            
        if (fileContent[j] == newline || fileContent[j] == space)
            continue;

        if (!isdigit(fileContent[j]))
            exit(EXIT_FAILURE);
        
        int result = 0;
        while (isdigit(fileContent[j])) {
            result = result * 10 + fileContent[j++] - zero;
        }
        
        
        if (result > 255)
            exit(EXIT_FAILURE);

        count--;
        j--;
    }

    // If count is != 0 by now, too many or too less entries are given.
    if (count)
        exit(EXIT_FAILURE);

    // Since everything is ok, save the values into the array data.
    int *data = malloc(width * height * 3 * sizeof(int));
    int index = 0;
    for (int j = i + 1; j <= fileSize; j++) {
        
        if (fileContent[j] == 0)
            break;
            
        if (fileContent[j] == 10 || fileContent[j] == space)
            continue;
        
        int result = 0;
        while (isdigit(fileContent[j])) {
            result = result * 10 + fileContent[j++] - zero;
        }

        data[index++] = result;
        j--;
    }

    // Set the global values for width and height, free the fileContent since it was allocated and return the data.
    WIDTH = width;
    HEIGHT = height;
    free(fileContent);
    return data;
}

/**
 * This function is used, to calculate the color difference (r, g, b) of two pixels.
 * @param r_1
 * @param g_1
 * @param b_1
 * @param r_2
 * @param g_2
 * @param b_2
 * @return color difference.
 */
int colorDifference(int r_1, int g_1, int b_1, int r_2, int g_2, int b_2) {
    return (r_1 - r_2) * (r_1 - r_2)  + (g_1 - g_2) * (g_1 - g_2) + (b_1 - b_2) * (b_1 - b_2);
}

/**
 * This function is called for the "-s" option. It prints the stats of the image onto the console.
 * @param data
 */
void computeStats(int* data) {

    // Calculate the brightness.
    int brightness = 0;
    int magnitude = 0;
    
    for (int i = 0; i < WIDTH * HEIGHT * 3; i = i + 3) {
        magnitude = data[i] + data[i + 1] + data[i + 2];
        magnitude /= 3;
        brightness += magnitude;
    }
    
    brightness /= WIDTH * HEIGHT;

    // Print the stats to the console.
    printf("width: %u\n", WIDTH);
    printf("height: %u\n", HEIGHT);
    printf("brightness: %u\n", brightness);
}

/**
 * This function is called for the "-p" option.
 * @param data
 * @param width
 * @param height
 * @return the minPath, as an X-value array, starting from the bottom of the image, going to the top.
 */
int* computeMinPath(int* data, int width) {

    // Resulting row - x values, the MINPATH.
    int *rowValues = malloc(HEIGHT * sizeof(int));

    // First compute the local Energy of every pixel.
    int localEnergy[width * HEIGHT];
    int index = 0;
    
    // width is decreasing but data has still the width WIDTH * 3.
    int rowLength = WIDTH * 3;
    // top -> bottom // left -> right
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < width * 3; x = x + 3) {
            // x + y * rowLength , this is the image coordinate. y = row, x = column -> (x, y)

            int cumulativeDistance = 0;

            // First the difference from the left, if existent.
            if (x >= 3) {
                cumulativeDistance += colorDifference(data[(x - 3) + y * rowLength], data[(x - 2) + y * rowLength], data[(x - 1) + y * rowLength],
                        data[x + y * rowLength], data[(x + 1) + y * rowLength], data[(x + 2) + y * rowLength]);
            }

            // Now the difference from the upper one, if existent.
            if (y > 0) {
                cumulativeDistance += colorDifference(data[x + (y - 1) * rowLength], data[(x + 1) + (y - 1) * rowLength], data[(x + 2) + (y - 1) * rowLength],
                        data[x + y * rowLength], data[(x + 1) + y * rowLength], data[(x + 2) + y * rowLength]);
            }

            // Save the computed local energy of the coordinate.
            localEnergy[index++] = cumulativeDistance;
        }
    }

    // Compute the fugen - energy now.
    // top -> bottom // left -> right
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < width; x++) {

            // If the coordinates exist, get the localEnergy value, otherwise assign INT_MAX.
            int upperLeft = (x > 0 && y > 0) ? localEnergy[(x - 1) + (y - 1) * width] : INT_MAX;
            int upperMid =  (y > 0) ? localEnergy[x + (y - 1) * width] : INT_MAX;
            int upperRight = (x < (width - 1)  && y > 0) ? localEnergy[(x + 1) + (y - 1) * width] : INT_MAX;

            // Now compute the minimum and add it to the localEnergy value if the minimum is not INT_MAX.
            int minimum = min(min(upperLeft, upperMid), upperRight);
            localEnergy[x + y * width] += (minimum == INT_MAX) ? 0 : minimum;
        }
    }

    // Go from right to left on the last row, determining the minimum value.
    int rowXValue = 0;
    int minimum = INT_MAX;
    int y = (HEIGHT - 1);


    // We use "<=" instead of "<" since we prefer lower x-values.
    for (int x = (width - 1); x >= 0; x--) {
        if (localEnergy[x + y * width] <= minimum) {
            minimum = localEnergy[x + y * width];
            rowXValue = x;
        }
    }


    // Now that we have the minimum of the last row, we only need to check the 3 upper ones till we reach the top.
    int idx = 0;
    rowValues[idx++] = rowXValue;
    while (y > 0) {
        y--;
        // Set it to INT_MAX if the coordinate does not exist.
        int rowXValue = rowValues[idx - 1];
        int upperRight = (rowXValue < width - 1) ? localEnergy[(rowXValue + 1) + y * width] : INT_MAX;
        int upperLeft = (rowXValue > 0) ? localEnergy[(rowXValue - 1) + y * width] : INT_MAX;
        int upperMid = localEnergy[rowXValue + y * width];
        
        // Priority is upperRight < upperLeft < upperMid.
        if (upperRight <= min(upperMid, upperLeft)) {
            rowValues[idx] = rowXValue + 1;
        }
        
        if (upperLeft <= min(upperMid, upperRight)) {
            rowValues[idx] = rowXValue - 1;
        }
        
        if (upperMid <= min(upperLeft, upperRight)) {
            rowValues[idx] = rowXValue;
        }

        idx++;
    }

    return rowValues;
}

/**
 * This function is used, to remove the rowValues entries from bottom to top out of the image data.
 * For every removed pixel, 0 0 0 (black pixel) is added at the right side of the image in the respective row.
 *
 * @param rowValues
 * @param data
 */
void removePixels(int* rowValues, int* data, int width) {

    // bottom -> top // left -> right
    int index = 0;
    for (int y = HEIGHT - 1; y >= 0; y--) {

        // Determine which column needs to be skipped.
        int skipColumn = rowValues[index++];
        
        int _x = 0;
        for (int x = 0; x < width * 3; x = x + 3) {
            // 3 Values are being skipped.
            if (skipColumn * 3 == x)
                continue;

            
            // Otherwise just copy the values over.
            data[_x + y * WIDTH * 3] = data[x + y * WIDTH * 3];
            data[(_x + 1) + y * WIDTH * 3] = data[(x + 1) + y * WIDTH * 3];
            data[(_x + 2) + y * WIDTH * 3] = data[(x + 2) + y * WIDTH * 3];
            _x = _x + 3;
        }
        // Set the remainding pixel values to 0.
        while(_x < WIDTH * 3) {
            data[_x + y * WIDTH * 3] = 0;
            _x++;
        }
    }
}

/**
 * This function is used, to print the computed values to the file "out.ppm".
 * @param data
 */
void writeDataToOut(int* data) {

    // Destination file of the resulting image should be out.ppm.
    FILE* f = fopen("out.ppm", "wb");
    // File could not be opened.
    if (f == NULL)
        exit(EXIT_FAILURE);

    // Write    P3
    //          WIDTH HEIGHT
    //          255
    //                      to the file.
    unsigned char twoFiveFive = 255;
    
    fprintf(f, "P3\n");
    fprintf(f, "%u %u\n", WIDTH, HEIGHT);
    fprintf(f, "%u\n", twoFiveFive);
    
    // fprintf(f, "P3\n%d %d\n255\n", WIDTH, HEIGHT);

    // Write the computed values to the file now.
    for (int i = 0; i < WIDTH * HEIGHT * 3; i++) {
        fprintf(f, "%u ", data[i]);
        
        if (i != 0 && i % (WIDTH * 3) == 0) {
            fprintf(f, "\n");
        }
    }

    // Close the file finally.
    fclose(f);
}


/**
 * Simple minimum function.
 * @param x
 * @param y
 * @return minimum of x and y.
 */
int min(int x, int y) {
    return x <= y ? x : y;
}
