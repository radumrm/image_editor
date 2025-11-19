//MARINCEA Radu-Mihail, 313CA. Tema 3
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Max number of words in one command
#define max_w 5
// Max number of characters in each word
#define max_l 30

typedef struct {
	// ok = 0 => No photo loaded or failed to load photo
	// ok = 1 => Photo loaded successfully
	int ok;
	// Matrix to store image data
	int **img;
	// Maximum intensity value
	int RGB;
	// Number of colors: colors = 1 => PGM, colors = 3 => PPM
	int colors;
	int rows, cols;
	// Selected coordinates for further processing
	int x1, x2, y1, y2;
	// Image format type
	char format_type[3];
} image;

typedef struct {
	// Number of words in each command
	int count;
	// Matrix to store all the words in a command
	char c[max_w][max_l];
} cmd;

// Function to split a command into separate words for processing
void read_command(cmd *commands)
{
	char aux[(max_w + 1) * max_l];
	fgets(aux, (max_w + 1) * max_l, stdin);
	commands->count = 0;
	char *p = strtok(aux, "\n ");
	while (p) {
		strcpy(commands->c[commands->count], p);
		p = strtok(NULL, "\n ");
		commands->count++;
	}
}

// Function to free the image matrix
void free_memory(image *a)
{
	if (a->ok == 0) {
		return; // Does nothing if no photo is succesfully loaded
	}
	a->ok = 0; // Makes the image counter 0 and frees the image
	for (int i = 0; i < a->rows; i++) {
		free(a->img[i]);
	}
	free(a->img);
}

/*  Function to allocate memory for the image matrix
	Returns 0 for success and 1 if there have been any errors */
int allocate_memory(image *a)
{
	a->img = malloc(a->rows * sizeof(int *));
	// Error handling
	if (!a->img) {
		free(a->img);
		return 1;	
	}
	for (int i = 0;  i < a->rows; i++) {
		/*  Allocating the columns * colors, to allocate the memory needed for
			either a PGM or PPM image => rows * cols for PGM,
			rows * (3 * cols) for PPM to store all 3 of the color channels */
		a->img[i] = malloc(a->cols * sizeof(int) * a->colors);
		// Error handling
		if (!a->img[i]) {
			while (i >= 0) {
				free(a->img[i]);
				i--;
			}
			free(a->img);
			return 1;
		}
	}
	// If it didn't fail earlier, the allocation is complete
	return 0;
}

/*  Function to read from a text, returns 0 for success, returns 1 if
	there are less pixels than needed (corrupted or incorrect file) */
int load_ascii(image *a, FILE *c)
{
	for (int i = 0; i < a->rows; i++) {
		for (int j = 0; j < a->cols * a->colors; j++) {
			if (!fscanf(c, "%d", &a->img[i][j])) {
				return 1;
			}
		}
	}
	return 0;
}

// Function to read from a binary file, same
int load_binary(image *a, FILE *in)
{
	unsigned char c; // unsigned char because it can hold values from [0, 255];
	for (int i = 0; i < a->rows; i++) {
		for (int j = 0; j < a->cols * a->colors; j++) {
			if (!fread(&c, sizeof(unsigned char), 1, in)) {
				return 1;
			}
			a->img[i][j] = (int)c;
		}
	}
	return 0;
}

// Function to load images
void load(image *a, char c[max_l])
{
	free_memory(a); // Free memory from previous image (if needed)
	FILE *in = fopen(c, "r");
	if (!in) {
		// Error handling
		printf("Failed to load %s\n", c);
		a->ok = 0; // Set the counter to 0 => no image loaded
		return;
	}
	// Reads the values present in ASCII format in all images
	fscanf(in, "%s%d%d%d", a->format_type, &a->cols, &a->rows, &a->RGB);
	a->colors = 1; // Sets the colors counter to 1 for PGM
	// Check if the format type is PPM, if it is colors = 3 for PPM
	if (a->format_type[1] == '3' || a->format_type[1] == '6') {
		a->colors = 3;
	}
	if (allocate_memory(a)) {
		printf("Malloc fail\n");
		return;
	}
	// Reads the entire line to skip over the newline character
	fgetc(in);
	int ok;
	// Checks the format type and chooses the correct loading function
	if (a->format_type[1] == '2' || a->format_type[1] == '3') {
		ok = load_ascii(a, in);
	} else if (a->format_type[1] == '5' || a->format_type[1] == '6') {
		ok = load_binary(a, in);
	}
	fclose(in);
	// Checks if loading is succesfull
	if (!ok) {
		// If succesfull, sets counter to 1
		// Assigns coordinates to cover all the image
		a->ok = 1;
		a->x1 = 0; a->x2 = a->cols;
		a->y1 = 0; a->y2 = a->rows;
		printf("Loaded %s\n", c);
	} else {
		// Else frees memory and sets counter to 0
		free_memory(a);
		a->ok = 0;
		printf("Failed to load %s\n", c);
	}
}

void swap(int *x, int *y)
{
	int tmp = *x;
	*x = *y;
	*y = tmp;
}

/*	Function to sort the pairs (x1, x2) and (y1, y2)
	As well as checking if the selection is correct
	Returns 1 if the selection is incorrect, 0 otherwise */
int sort_and_verif(int *a, int *b, int c)
{
	if (*a == *b) {
		return 1;
	}
	if (*a > *b) {
		swap(a, b);
	}
	if (*a < 0 || *b > c)
		return 1;
	return 0;
}

// Function to select coordinates for further image processing
void selectt(image *a, cmd commands)
{
	// No image loaded previously
	if (a->ok == 0) {
		printf("No image loaded\n");
		return;
	}
	// Command doesn't have the correct ammount of parameters
	if (commands.count != 2 && commands.count != 5) {
		printf("Invalid command\n");
		return;
	}
	// Select ALL
	if (commands.count == 2 && !strcmp(commands.c[1], "ALL")) {
		a->x1 = 0; a->x2 = a->cols;
		a->y1 = 0; a->y2 = a->rows;
		printf("Selected ALL\n");
	} else if (commands.count == 5) {
		// SELECT (x1, y1) (x2, y2)
		int x1_tmp, y1_tmp, x2_tmp, y2_tmp;
		// Using temporary variables to not corrupt last pair of coordinates
		x1_tmp = atoi(commands.c[1]); x2_tmp = atoi(commands.c[3]);
		y1_tmp = atoi(commands.c[2]); y2_tmp = atoi(commands.c[4]);
		// Checks to see if each one is a number, not a random string
		if (!x1_tmp && strcmp(commands.c[1], "0")) {
			printf("Invalid command\n");
			return;
		}
		if (!x2_tmp && strcmp(commands.c[3], "0")) {
			printf("Invalid command\n");
			return;
		}
		if (!y1_tmp && strcmp(commands.c[2], "0")) {
			printf("Invalid command\n");
			return;
		}
		if (!y2_tmp && strcmp(commands.c[4], "0")) {
			printf("Invalid command\n");
			return;
		}
		// Uses previous sort_and_verif function
		int tmp1 = sort_and_verif(&x1_tmp, &x2_tmp, a->cols);
		int tmp2 = sort_and_verif(&y1_tmp, &y2_tmp, a->rows);
		if (tmp1 || tmp2) {
			printf("Invalid set of coordinates\n");
		} else {
			// Assigns new selection
			a->x1 = x1_tmp; a->x2 = x2_tmp; a->y1 = y1_tmp; a->y2 = y2_tmp;
			printf("Selected %d %d %d %d\n", a->x1, a->y1, a->x2, a->y2);
		}
	}
}

// Function to CROP an image
void crop(image *a)
{
	// No image loaded previously
	if (a->ok == 0) {
		printf("No image loaded\n");
		return;
	}

	// Variables to store new image size
	int dif_x = a->x2 - a->x1;
	int dif_y = a->y2 - a->y1;

	int **aux = malloc(dif_y * sizeof(int *));
	// Error handling
	if (!aux) {
		free(aux);
		return;
	}
	for (int i = 0; i < dif_y; i++) {
		aux[i] = malloc(dif_x * sizeof(int) * a->colors);
		if (!aux[i]) {
			while (i >= 0) {
				free(aux[i]);
				i--;
			}
			return;
		}
		// In the aux matrix we will save the cropped image
		for (int j = 0; j < dif_x * a->colors; j++) {
			aux[i][j] = a->img[i + a->y1][a->x1 * a->colors + j];
		}
	}
	free_memory(a); // Free uncropped image
	a->ok = 1; // Sets counter to 1
	a->img = aux; // img becomes the cropped image (aux)
	a->cols = dif_x; a->rows = dif_y;
	a->x1 = 0; a->x2 = a->cols;
	a->y1 = 0; a->y2 = a->rows;
	printf("Image cropped\n");
}

// Function to save img data as text
void save_ascii(image *a, char c[max_l])
{
	FILE *out = fopen(c, "w");
	if (!out) {
		return;
	}
	if (a->colors == 1) {
		fprintf(out, "P2\n");
	} else {
		fprintf(out, "P3\n");
	}
	fprintf(out, "%d %d\n%d\n", a->cols, a->rows, a->RGB);
	for (int i = 0; i < a->rows; i++) {
		for (int j = 0; j < a->cols * a->colors; j++) {
			fprintf(out, "%d ", a->img[i][j]);
		}
		fprintf(out, "\n");
	}
	fclose(out);
}

// Function to save img data as binary
void save_binary(image *a, char c[max_l])
{
	FILE *out = fopen(c, "wb");
	if (!out) {
		return;
	}
	if (a->colors == 1) {
		fprintf(out, "P5\n");
	} else {
		fprintf(out, "P6\n");
	}
	fprintf(out, "%d %d\n%d\n", a->cols, a->rows, a->RGB);
	for (int i = 0; i < a->rows; i++) {
		for (int j = 0; j < a->cols * a->colors; j++) {
			fwrite(&a->img[i][j], sizeof(unsigned char), 1, out);
		}
	}
	fclose(out);
}

// Function to save image after editing it
void save(image *a, cmd commands)
{
	// No image loaded previously
	if (a->ok == 0) {
		printf("No image loaded\n");
		return;
	}
	// Save the image according to the specified command
	if (commands.count == 3 && !strcmp(commands.c[2], "ascii")) {
		save_ascii(a, commands.c[1]);
		printf("Saved %s\n", commands.c[1]);
	} else if (commands.count == 2) {
		save_binary(a, commands.c[1]);
		printf("Saved %s\n", commands.c[1]);
	}
}

// Function for edge cases, more in README
void assign_values_for_edge(image *a, int *x1, int *x2, int *y1, int *y2)
{
	*x1 = 0; *x2 = 0; *y1 = 0; *y2 = 0;
	if (a->x1 == 0) {
		*x1 = 1;
	}
	if (a->y1 == 0) {
		*y1 = 1;
	}
	if (a->x2 == a->cols) {
		*x2 = 1;
	}
	if (a->y2 == a->rows) {
		*y2 = 1;
	}
}

// Round function, roundd(9.5) = 10; roundd(9.49) = 9;
int roundd(double a)
{
	int aux;
	aux = (int)a;
	if (a >= aux + 0.5) {
		return aux + 1;
	}
	return aux;
}

// Function to verify error cases before kernel operations
int verif_apply(image *a, cmd commands)
{
	if (a->ok == 0) {
		printf("No image loaded\n");
		return 1;
	}
	if (commands.count != 2) {
		printf("Invalid command\n");
		return 1;
	} else if (a->format_type[1] == '2' || a->format_type[1] == '5') {
		printf("Easy, Charlie Chaplin\n");
		return 1;
	}
	return 0;
}

// Function to apply different filters (kernels) for PPM images, more in README
void apply(image *a, cmd commands)
{
	if (verif_apply(a, commands)) {
		return;
	}
	int prm = -1; // Counter for which kernel to use in the apl 3d array
	double apl[4][4][3] = {{{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}, {1, 0, 0}},
						{{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}, {1, 0, 0}},
						{{1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {9, 0, 0}},
						{{1, 2, 1}, {2, 4, 2}, {1, 2, 1}, {16, 0, 0}}};
	if (!strcmp(commands.c[1], "EDGE")) { // Parameter selection
		prm = 0;
	} else if (!strcmp(commands.c[1], "SHARPEN")) {
		prm = 1;
	} else if (!strcmp(commands.c[1], "BLUR")) {
		prm = 2;
	} else if (!strcmp(commands.c[1], "GAUSSIAN_BLUR")) {
		prm = 3;
	}
	if (prm == -1) {
		printf("APPLY parameter invalid\n");
		return;
	} // Creating a new matrix where we will apply the filter
	int dif_x = a->x2 - a->x1;
	int dif_y = a->y2 - a->y1;
	int **aux = malloc(dif_y * sizeof(int *));
	if (!aux) {
		free(aux);
		return;
	}
	for (int i = 0; i < dif_y; i++) {
		aux[i] = malloc(dif_x * sizeof(int) * a->colors);
		if (!aux[i]) {
			while (i >= 0) {
				free(aux[i]);
				i--;
			}
			return;
		}
	} // More about edge variables in read me
	int edge_x1, edge_x2, edge_y1, edge_y2;
	assign_values_for_edge(a, &edge_x1, &edge_x2, &edge_y1, &edge_y2);
	for (int i = edge_y1; i < dif_y - edge_y2; i++) {
		for (int j = edge_x1; j < dif_x - edge_x2; j++) {
			for (int k = 0; k < a->colors; k++) {
				int s = 0;
				for (int l = -1; l < 2; l++) {
					for (int m = -1; m < 2; m++) {
						int ni = i + a->y1;
						int nj = j + a->x1;
						int pixel = a->img[ni + l][(nj + m) * a->colors + k];
						int ker_val = apl[prm][l + 1][m + 1];
						s += pixel * ker_val;
					}
				}
				double sum = s / apl[prm][3][0];
				if (sum > a->RGB) {
					aux[i][j * a->colors + k] = a->RGB;
				} else if (sum < 0) {
					aux[i][j * a->colors + k] = 0;
				} else {
					aux[i][j * a->colors + k] = roundd(sum);
				}
			}
		}
	}
	for (int i = edge_y1; i < dif_y - edge_y2; i++) {
		for (int j = edge_x1; j < dif_x - edge_x2; j++) {
			for (int k = 0; k < a->colors; k++) {
				int ni = i + a->y1;
				int nj = j + a->x1;
				a->img[ni][nj * a->colors + k] = aux[i][j * a->colors + k];
			}
		}
	}
	for (int i = 0; i < dif_y; i++) { // Freeing the aux matrix
		free(aux[i]);
	}
	free(aux);
	printf("APPLY %s done\n", commands.c[1]);
}

// Function to equalize the PGM images
void equalize(image *a)
{
	// No image loaded previously
	if (!a->ok) {
		printf("No image loaded\n");
		return;
	}
	if (a->colors == 3) {
		printf("Black and white image needed\n");
		return;
	}
	// Storing the frequency of each value into the f array
	int f[256] = {0};
	for (int i = 0; i < a->rows; i++) {
		for (int j = 0; j < a->cols; j++) {
			f[a->img[i][j]]++;
		}
	}

	// Storing the sum of frequencies of all previous values
	int s_f[256] = {0};
	s_f[0] = f[0]; // initializing it with f[0]
	for (int i = 1; i < 256; i++) {
		s_f[i] += s_f[i - 1] + f[i];
	}

	// Applying equalize algorithm
	double area = a->rows * a->cols;
	for (int i = 0; i < a->rows; i++) {
		for (int j = 0; j < a->cols; j++) {
			float aux = (255.0 * s_f[a->img[i][j]]) / area;
			// Clamp function
			if (aux > a->RGB) {
				a->img[i][j] = a->RGB;
			} else {
				a->img[i][j] = roundd(aux);
			}
		}
	}
	printf("Equalize done\n");
}

// Function to check if a number is a power of 2
int is_pow(int y)
{
	int k = 0;
	if (y > 256) {
		return 1;
	}
	while (1 << k <= y) {
		if (1 << k == y) {
			return 0;
		}
		k++;
	}
	return 1;
}

// Function to print the histogram
void histogram(image *a, cmd commands)
{
	// No image loaded previously
	if (!a->ok) {
		printf("No image loaded\n");
		return;
	}
	if (commands.count != 3) {
		printf("Invalid command\n");
		return;
	}
	// Verifying if the parameters are correct
	int x = atoi(commands.c[1]);
	int y = atoi(commands.c[2]);
	if (!y || !x || is_pow(y)) {
		printf("Invalid set of parameters\n");
		return;
	}
	if (a->colors == 3) {
		printf("Black and white image needed\n");
		return;
	}
	// Storing the frequency of each value into the f array
	int f[256] = {0};
	// Storing the added frequencies of all values in the specific bin
	int s_f[256] = {0};
	// The number of elements in each bin
	int elements = 256 / y;
	// The maximum value of a bin
	int max = 0;
	for (int i = 0; i < a->rows; i++) {
		for (int j = 0; j < a->cols; j++) {
			f[a->img[i][j]]++;
		}
	}
	// For each bin, add the frequencies into s_f[i]
	// And finding the maximum value of a bin
	for (int i = 0; i < y; i++) {
		int ok = 0;
		while (ok < elements) {
			s_f[i] += f[ok + i * elements];
			ok++;
		}
		if (s_f[i] > max) {
			max = s_f[i];
		}
	}

	// Appling the hisogram algorithm
	for (int i = 0; i < y; i++) {
		// Storing the number of stars into aux
		int aux = (s_f[i] * x) / max;
		fprintf(stdout, "%d\t|\t", aux);
		for (int j = 0; j < aux; j++) {
			fprintf(stdout, "*");
		}
		fprintf(stdout, "\n");
	}
}

// Function to rotate a square selection or the whole image to the right
void rotate_right(image *a)
{
	int dif_x = a->x2 - a->x1;
	int dif_y = a->y2 - a->y1;
	// Allocating memory for a matrix of cols * (rows * colors) dimension
	int **aux = malloc(dif_x * sizeof(int *));
	if (!aux) {
		free(aux);
		return;
	}
	for (int i = 0; i < dif_x; i++) {
		aux[i] = malloc(dif_y * a->colors * sizeof(int));
		if (!aux[i]) {
			while (i >= 0) {
				free(aux[i]);
				i--;
			}
			return;
		}
		// Copying the values of the image into the rotated image aux
		for (int j = 0; j < dif_y; j++) {
			for (int k = 0; k < a->colors; k++) {
				int nj1 = a->colors * j + k;
				int nj2 = a->colors * (i + a->x1) + k;
				aux[i][nj1] = a->img[a->y2 - 1 - j][nj2];
			}
		}
	}
	/*  If the selection is the whole image, img will become aux
		And the selecion as well as the number of rows and cols
		Will have to swap values */
	if (a->x1 == 0 && a->x2 == a->cols && a->y1 == 0 && a->y2 == a->rows) {
		free_memory(a); a->ok = 1;
		swap(&a->y2, &a->x2);
		swap(&a->rows, &a->cols);
		a->img = aux;
	} else {
		// Else we copy into the square selection of the image
		// The coresponding values from aux matrix
		for (int i = 0; i < dif_y; i++) {
			for (int j = 0; j < a->colors * dif_x; j++) {
				int ni = i + a->y1;
				int nj = j + a->colors * a->x1;
				a->img[ni][nj] = aux[i][j];
			}
		}
		// And than free aux
		for (int i = 0; i < dif_x; i++) {
			free(aux[i]);
		}
		free(aux);
	}
}

/*  Function to convert the angle into a value that indicates
	how many rotations to the right the image must do */
int angle_converter(int angle)
{
	if (angle == 0 || angle == 360 || angle == -360) {
		return 0;
	}
	if (angle > 0) {
		return angle / 90;
	}
	if (angle < 0) {
		return 4 + angle / 90;
	}
	return -1;
}

/*  Function to rotate a square selection
	or the whole image to a specified angle */
void rotate(image *a, cmd commands)
{
	if (!a->ok) {
		printf("No image loaded\n");
		return;
	}
	if (commands.count != 2) {
		printf("Invalid command\n");
		return;
	}
	int angle = atoi(commands.c[1]);
	if (!angle && strcmp(commands.c[1], "0")) {
		printf("Invalid command\n");
		return;
	}
	if (angle % 90 != 0 || angle < -360 || angle > 360) {
		printf("Unsupported rotation angle\n");
		return;
	}
	if (!(a->x1 == 0 && a->x2 == a->cols && a->y1 == 0 && a->y2 == a->rows)) {
		if ((a->x2 - a->x1) != (a->y2 - a->y1)) {
			printf("The selection must be square\n");
			return;
		}
	}
	for (int i = 0; i < angle_converter(angle); i++) {
		rotate_right(a);
	}
	printf("Rotated %d\n", angle);
}

/*  The function processes the commands after they have
	been split into different words in read_command function
	and calls for the appropriate function */
void start(cmd commands, image *a)
{
	if (!strcmp(commands.c[0], "LOAD")) {
		load(a, commands.c[1]);
		return;
	} else if (!strcmp(commands.c[0], "SELECT")) {
		selectt(a, commands);
		return;
	} else if (!strcmp(commands.c[0], "CROP")) {
		crop(a);
		return;
	} else if (!strcmp(commands.c[0], "SAVE")) {
		save(a, commands);
		return;
	} else if (!strcmp(commands.c[0], "APPLY")) {
		apply(a, commands);
		return;
	} else if (!strcmp(commands.c[0], "EQUALIZE")) {
		equalize(a);
		return;
	} else if (!strcmp(commands.c[0], "HISTOGRAM")) {
		histogram(a, commands);
		return;
	} else if (!strcmp(commands.c[0], "ROTATE")) {
		rotate(a, commands);
		return;
	}
	printf("Invalid command\n");
}

int main(void)
{
	// Creating a cmd variable
	cmd commands;
	// Reading the first command
	read_command(&commands);

	// Creating an image variable
	image a;
	// Initializing the counter to 0
	a.ok = 0;
	/*  Processing the command and reading the next one
		until EXIT command */
	while (strcmp(commands.c[0], "EXIT")) {
		start(commands, &a);
		read_command(&commands);
	}
	// If there is no image loaded upon exit
	if (a.ok == 0) {
		printf("No image loaded\n");
	}
	// Freeing memory if needed
	free_memory(&a);
	return 0;
}

