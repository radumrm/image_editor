OVERVIEW:
---------------------------------------------------------------------------
This is a program designed for editing images with a PPM or PGM format
for both ASCII and binary files. It can do a variety of operations such as:
- Loading ASCII or Binary photos
- Selecting regions of an image for editing
- Cropping
- Applying different filters (kernels) such as:
    -> EDGE for edge detection
    -> SHARPEN for higher quality images by enhancing edges
    -> BLUR to reduce image sharpenss
    -> GAUSSIAN_BLUR a more advanced blurring technique
- Equalizing the PGM format images, correcting overexposure and enhancing
the visibility of details that might be hard to notice otherwise
- Histogram
- Saving photos in ASCII format as well as binary

STRUCTURES:
---------------------------------------------------------------------------
The program uses 2 structures, image and cmd.
Image structure holds the information about the image such as:
- ok (counter for loaded photo)
- img (matrix to store image data)
- RGB (maxmium intensity value)
- colors (the number of color channels, 1->PGM, 3->PPM)
- rows, cols (number of rows and columns for pixels)
- x1,y1 ; x2,y2 (selected coordinates for operations)
- format_type

CMD structure holds the information for each command line:
- count (number of words in the command)
- c (matrix to store the individual words of a command)

COMMANDS:
---------------------------------------------------------------------------
LOAD file: loading PPM and PGM files in binary or ASCII format
SELECT x1 y1 x2 y2: selecting only a part of the image to do operation on
SELECT ALL: selecting the whole image
CROP: reducing the whole image to the current selection
APPLY FILTER: applying filters on the current selection
EQUALIZE: correcting overexposure
HISTOGRAM x y: printing the histogram using maximum x starts and y bins
ROTATE angle: rotating selection to an angle in the [-360, 360] range
SAVE file: saving the file in binary format
SAVE file ascii: saving the file in ASCII format

FUNCTIONS:
---------------------------------------------------------------------------
void read_command: splits a command into separate words

void free_memory: frees img data

int allocate_memory: allocates appropriate memory for the image data, PGM or 
PPM(using colors; colors = 1 PGM; colors = 3 PPM).
Returns 1 if allocation failed, 0 if succedeed.

int load_ascii: reads the image data in ascii form
int load_binary: reads the image data in binary form
both return 1 if there are less pixels than needed and 0 for success
void load: frees memory if there was an image loaded previously,
reads the format, cols, rows, and RGB. Initializez the colors to 1, then
checks if the format is PPM, changing the colors value. Uses fget to read
\n character. Than makes selection of which funtion to choose: load_ascii
or load_binary. Also initializezsthe x1,y1,x2,y2 to include the whole image.

int sort_and_verif: sorts the values of (x1, x2) and (y1, y2) pair and also
checks if the selection is within the image. It returns 1 if the selection is
incorrect or 0 if the selection is correct
void selectt: selects the whole image or a sub-image, if it passes sort_and_verif
then the new selection is assigned

void crop: allocates memory for the new cropped image. Frees previous image and 
the previous image becomes aux;

void save_ascii: saves the edited image into ascii format
void save_binary: saves the image into binary format
void save: reads the command and selects between void_ascii
and void_binary.

void assign_values_for_edges: verifies all edge cases and saves into edge_x1_y1
_x2_y2 either 1 or 0. 1 if the selection is on the edge, and 0 if the selection is 
inside the picture (has all 8 neighbors).
void roundd: rounds a double, returning an int
int verif_apply: checks for error cases
void apply: firstly, the program checks if there are not any error cases.
initializes the prm to -1 and creates the 3d array apl
in the 3d array apl, the first three 1d arrays represent the kernel values
the first digit in the last 1d array represent the number you have to divide
the sum with. The program then chooses the appropriate value for prm, and also
checks if the prm is still -1, meaning incorrect parameter. Then it allocates
space for aux, an int matrix where we will temporarily store the values after
the appropriate calculations. Then we will assign the appropriate values for
edge_x1_x2_y1_y2 so we can go through the pixels of the image which have 8
neighbors. Than we use a temporary variable s to sum up the appropriate kernel
calculations. Than into the sum variable we divide s to the appropriate value.
We use clamp function on the sum variable and store it's value into aux.
After that we go through each pixel of aux and copying it's value into the original
matrix img. Then we free aux.

equalize: stores the frequency of each value into the f array. Initializes
s_f[0] to f[0]. Then goes through all the values of the pixels 0->255 
storing the sum of frequencies of all previous values in s_f[i] 
(s_f[i] = f[0] + f[1] + .... + f[i]). Than we apply the equalize algorithm,
storing the temporary value into aux, and then using clamp function.

int is_pow: verifies if a number is a power of 2 using bit calculations
void histogram: checks if the selected numbers are correct for the calculation
stores the frequency of each value into the f array. Then goes through each
bin and adds into s_f[i] the sum of all f[values], in that bin. It also checks
for the maximum after each calculation. Then it applies the algorithm.

void rotate_right: allocates memory for a matrix of cols * (rows * colors) dimensions.
I calculated the rotated image in aux. I check if the selection is the
entire image, if it is I free the last image data, and img becomes aux, but i also
swap the selections, columns and rows, because that means it becomes it's transpose.
If not that means the selection is square, I go through each pixel in img and put in
the corresponding values from the aux matrix, tean I free aux.

void start: checks the 1st word of the command and calls of the appropriate function

int main:
I create the cmd variable commands. I read and memorize the first comand using
read_command. I create the image variable and initialize it's counter to 0
(no image loaded). I go through a loop where I process a command than read the next
command until EXIT command. I than verify if a image was laoded prior to EXIT and
free the memory (if needed).
