#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int convert_one_sample(double factor, double *curr_factor);
int fin_convert_one_line(double factor, double *curr_factor, int channel);
int fout_convert_one_line(double factor, double *curr_factor2, long int *samples_that_dont_need_effect, int channel);
int pan_convert_one_line(double factor, double *curr_factor, double *curr_factor2);

int main(int argc, char **argv) {
	
	if (feof(stdin)) {
		printf("Error: you need to input a hex dump to stdin for this program to work");
		return 0;
	}
	
	//check for valid input
	if (argc != 3) {
		printf("\nError: Invalid command-line arguments.\n");
		return 0;
	}

	if (feof(stdin)) {
		printf("Error: need a file");
		return 0;
	}

	//extract the effect type and effect duration from the input
	double effect_duration = ((double)strtol(argv[2], NULL, 10))/1000.0;
	if (effect_duration < 0) {
		printf("Error: cannot have negative duration");
		return 0;
	}
	char effect_type[6];
	strncpy(effect_type, argv[1], 6);

	//read and print first line
	char a[77];
	if (fgets(a, 77, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", a);

	//read and print second line 
	char b[29];
	if (fgets(b, 29, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", b);
	
	//check if file is mono or stero
	char channel[7];
	if (fgets(channel, 7, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", channel);

	int num_channels;
	if (channel[1] == '2') {
		num_channels = 2;
	}
	else if (channel[1] == '1') {
		num_channels = 1;
	}
	else {
		printf("\nError: invalid number of channels\n");
		return 0;
	}

	//find smaple rate byte-by-byte
	char sample_rate_string[9];

	char sample_rate_byte_1[4];
	if (fgets(sample_rate_byte_1, 4, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", sample_rate_byte_1);

	char sample_rate_byte_2[4];
	if (fgets(sample_rate_byte_2, 4, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", sample_rate_byte_2);

	char sample_rate_byte_3[4];
	if (fgets(sample_rate_byte_3, 4, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", sample_rate_byte_3);

	char sample_rate_byte_4[4];
	if (fgets(sample_rate_byte_4, 4, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", sample_rate_byte_4);
	
	//make the string representation of the sample rate
	strncpy(sample_rate_string, sample_rate_byte_4, 2);
	sample_rate_string[2] = '\0';
	strncat(sample_rate_string, sample_rate_byte_3, 2);
	sample_rate_string[4] = '\0';
	strncat(sample_rate_string, sample_rate_byte_2, 2);
	sample_rate_string[6] = '\0';
	strncat(sample_rate_string, sample_rate_byte_1, 2);
	sample_rate_string[8] = '\0';
	
		//convert string sample rate to int sample rate
	double sample_rate_int = (double) strtol(sample_rate_string, NULL, 16); 
	
	//number of samples that need effect
	double samples_that_need_effect = sample_rate_int * effect_duration;

	//starting factor to multiply the sample values by
	double factor;
	if (samples_that_need_effect == 0) {
		factor = 1.0;
	}
	else {
		factor = 1.0/((double) samples_that_need_effect);
	}

	//initialize the current factor that goes up from 0
	double curr_factor;
	if (samples_that_need_effect == 0) {
		curr_factor = 1.0;
	} 
	else {
		curr_factor = 0.0;
	}

	//initialize the current factor that goes down from 1
	double curr_factor2;
	if (samples_that_need_effect == 0) {
		curr_factor2 = 0.0;
	}
	else {
		curr_factor2 = 1.0 - factor;
	}


	//read and print the rest of the header
	char rest_of_header_1[31];
	if (fgets(rest_of_header_1, 31, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", rest_of_header_1);

	char rest_of_header_2[35];
	if (fgets(rest_of_header_2, 35, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", rest_of_header_2);
	
	//getting number of samples/data size
	char data_size[9];

	char b1[4];
	if (fgets(b1, 4, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", b1);

	char b2[4];
	if (fgets(b2, 4, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", b2);

	char b3[4];
	if (fgets(b3, 4, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", b3);

	char b4[4];
	if (fgets(b4, 4, stdin) == NULL) {
		printf("Error: error processing file");
		return 0;
	}
	printf("%s", b4);

	strncpy(data_size, b4, 2);
	data_size[2] = '\0';
	strncat(data_size, b3, 2);
	data_size[4] = '\0';
	strncat(data_size, b2, 2);
	data_size[6] = '\0';
	strncat(data_size, b1, 2);
	data_size[8] = '\0';

	long int total_samples;
	if (num_channels == 1) {
		total_samples = (long int) strtol(data_size, NULL, 16)/2;
	}
	else {
		total_samples = (long int) strtol(data_size, NULL, 16)/4;
	}

	//calculate samples that dont need effects, used to determine where to start the fout effect
	long int samples_that_dont_need_effect = total_samples - samples_that_need_effect;

	//FADE-IN EFFECT
	if (strncmp(argv[1], "-fin", 4) == 0) {
		if (num_channels == 1) {

			//convert first sample
			short int outcome = convert_one_sample(factor, &curr_factor);
			if (outcome == 1) {
				return 0;
			}
			else if (outcome == -1) {
				printf("\nError processing file\n");
				return 0;
			}	
			else {
				if (curr_factor < 1) {
					curr_factor = curr_factor + factor;
				}
			}

			//convert second sample
			outcome = convert_one_sample(factor, &curr_factor);
			if (outcome == 1) {
				return 0;
			}
			else if (outcome == -1) {
				printf("\nError processing file\n");
				return 0;
			}
			else {
				if (curr_factor < 1) {
					curr_factor = curr_factor + factor;
				}
			}

			char rest_of_line[19];
			if (fgets(rest_of_line, 19, stdin) == NULL) {
				if (feof(stdin)) {
					return 1;
				}
				else if (ferror(stdin)) {
					return -1;
				}
			}
			printf("%s", rest_of_line);

			//convert rest of the dump
			while (outcome == 0) {
				outcome = fin_convert_one_line(factor, &curr_factor, num_channels);
			}
			if (outcome == -1) {
				printf("\nError processing file\n");
				return 0;
			}
			return 0;

		}
		else if (num_channels == 2) {

			//convert first sample
			short int outcome = convert_one_sample(factor, &curr_factor);
			if (outcome == 1) {
				return 0;
			}
			else if (outcome == -1) {
				printf("\nError processing file\n");
				return 0;
			}	

			//convert second sample
			outcome = convert_one_sample(factor, &curr_factor);
			if (outcome == 1) {
				return 0;
			}
			else if (outcome == -1) {
				printf("\nError processing file\n");
				return 0;
			}
			else {
				if (curr_factor < 1) {
					curr_factor = curr_factor + factor;
				}
			}

			char rest_of_line[19];
			if (fgets(rest_of_line, 19, stdin) == NULL) {
				if (feof(stdin)) {
					return 1;
				}
				else if (ferror(stdin)) {
					return -1;
				}
			}
			printf("%s", rest_of_line);

			//convert rest of the dump
			while (outcome == 0) {
				outcome = fin_convert_one_line(factor, &curr_factor, num_channels);
			}
			if (outcome == -1) {
				printf("\nError processing file\n");
				return 0;
			}
			return 0;
		}
	}


	//FADE-OUT EFFECT
	else if (strncmp(argv[1], "-fout", 4) == 0) {
		short int outcome;
		if (num_channels == 1) {
			if (samples_that_dont_need_effect <= 0) {

				outcome = convert_one_sample(factor, &curr_factor2);
				if (outcome == 1) {
					return 0;
				}
				else if (outcome == -1) {
					printf("\nError processing file\n");
					return 0;
				}	
				curr_factor2 = curr_factor2 - factor;

				outcome = convert_one_sample(factor, &curr_factor2);
				if (outcome == 1) {
					return 0;
				}
				else if (outcome == -1) {
					printf("\nError processing file\n");
				}
				curr_factor2 = curr_factor2 - factor;	
			}
			else {
				double garbage_factor = 1.0;
				double garbage_curr_factor = 1.0;

				outcome = convert_one_sample(garbage_factor, &garbage_curr_factor);
				if (outcome == 1) {
					return 0;
				}
				else if (outcome == -1) {
					printf("\nError processing file\n");
					return 0;
				}
				samples_that_dont_need_effect = samples_that_dont_need_effect - 1;

				if (samples_that_dont_need_effect <= 0) {

					outcome = convert_one_sample(factor, &curr_factor2);
					if (outcome == 1) {
						return 0;
					}
					else if (outcome == -1) {
						printf("\nError processing file\n");
						return 0;
					}
					curr_factor2 = curr_factor2 - factor;
				}
				else {
					outcome = convert_one_sample(garbage_factor, &garbage_curr_factor);
					if (outcome == 1) {
						return 0;
					}
					else if (outcome == -1) {
						printf("\nError processing file\n");
						return 0;
					}
					samples_that_dont_need_effect = samples_that_dont_need_effect - 1;
				}

			}

			char rest_of_line[19];
			if (fgets(rest_of_line, 19, stdin) == NULL) {
				if (feof(stdin)) {
					return 1;
				}
				else if (ferror(stdin)) {
					return -1;
				}
			}
			printf("%s", rest_of_line);

			while (outcome == 0) {
				outcome = fout_convert_one_line(factor, &curr_factor2, &samples_that_dont_need_effect, num_channels);
			}
			if (outcome == -1) {
				printf("\nError processing file\n");
				return 0;
			}
		}




		else {
			if (samples_that_dont_need_effect <= 0) {

				outcome = convert_one_sample(factor, &curr_factor2);
				if (outcome == 1) {
					return 0;
				}
				else if (outcome == -1) {
					printf("\nError processing file\n");
					return 0;
				}

				outcome = convert_one_sample(factor, &curr_factor2);
				if (outcome == 1) {
					return 0;
				}
				else if (outcome == -1) {
					printf("\nError processing file\n");
					return 0;
				}
				curr_factor2 = curr_factor2 - factor;

			}
			else {

				double garbage_factor = 1.0;
				double garbage_curr_factor = 1.0;

				outcome = convert_one_sample(garbage_factor, &garbage_curr_factor);
				if (outcome == 1) {
					return 0;
				}
				else if (outcome == -1) {
					printf("\nError processing file\n");
					return 0;
				}

				outcome = convert_one_sample(garbage_factor, &garbage_curr_factor);
				if (outcome == 1) {
					return 0;
				}
				else if (outcome == -1) {
					printf("\nError processing file\n");
					return 0;
				}
				samples_that_dont_need_effect = samples_that_dont_need_effect - 1;
			}

			char rest_of_line[19];
			if (fgets(rest_of_line, 19, stdin) == NULL) {
				if (feof(stdin)) {
					return 1;
				}
				else if (ferror(stdin)) {
					return -1;
				}
			}
			printf("%s", rest_of_line);

			while (outcome == 0) {
				outcome = fout_convert_one_line(factor, &curr_factor2, &samples_that_dont_need_effect, num_channels);
			}
			if (outcome == -1) {
				printf("\nError processing file\n");
				return 0;
			}
		}
	}


	//PAN EFFECT
	else if (strncmp(argv[1], "-pan", 4) == 0) {

		//make sure file is stero
		if (num_channels == 1) {
			printf("\nError: -pan only works with stero wav files\n");
			return 0;
		}

		//read first byte and multiply it with the fout factor
		short int outcome = convert_one_sample(factor, &curr_factor2);
		if (outcome == 1) {
			return 0;
		}
		else if (outcome == -1) {
			printf("\nError processing file\n");
			return 0;
		}	
		else {
			if (curr_factor2 > 0) {
				curr_factor2 = curr_factor2 - factor;
			}
		}

		//read second byte and multiply it with the fin factor
		outcome = convert_one_sample(factor, &curr_factor);
		if (outcome == 1) {
			return 0;
		}
		else if (outcome == -1) {
			printf("\nError processing file\n");
			return 0;
		}	
		else {
			if (curr_factor < 1) {
				curr_factor = curr_factor + factor;
			}
		}

		char rest_of_line[19];
		if (fgets(rest_of_line, 19, stdin) == NULL) {
			if (feof(stdin)) {
				return 1;
			}
			else if (ferror(stdin)) {
				return -1;
			}
		}
		printf("%s", rest_of_line);

		//read rest of the file
		while (outcome == 0) {
			outcome = pan_convert_one_line(factor, &curr_factor, &curr_factor2);
		}
		if (outcome == -1) {
			printf("\nError processing file\n");
			return 0;
		}
		return 0;

	}
	else {
		printf("Error: invalid effect");
		return 0;
	}
}
/*
convert a single sample of from stdin to the correct value.

RETURN VALUES
- return 0 upon successful convertion
- return 1 upon EOF before convertion can happen
- return -1 upon error before convertion can happen
*/
int convert_one_sample(double factor, double *curr_factor) {

	if (*curr_factor < 0.0) {
		*curr_factor = 0.0;
	}
	if (*curr_factor > 1.0) {
		*curr_factor = 1.0;
	}

	char sample_string[5];

	//extract first byte and check for EOF or error
	char sample_string_b1[4];
	if (fgets(sample_string_b1, 4, stdin) == NULL) {
		
		if (feof(stdin)) {
			return 1;
		}

		else if (ferror(stdin)) {
			return -1;
		}
	}

	//extract second byte and check for EOF or error
	char sample_string_b2[4];
	if (fgets(sample_string_b2, 4, stdin) == NULL) {
		
		if (feof(stdin)) {
			return 1;
		}

		else if (ferror(stdin)) {
			return -1;
		}
	}

	//adjust left and right sample strings for endianess
	strncpy(sample_string, sample_string_b2, 2);
	sample_string[2] = '\0';
	strncat(sample_string, sample_string_b1, 2);
	sample_string[4] = '\0';
	
	char spaces[5] = "    ";
	if (strcmp(sample_string, spaces) == 0) {
		printf("      ");
		return 0;
	}
	
	for (int i = 0; i < 4; i++) {
		if (sample_string[i] == ' ') {
			return -1;
		}
	}
	
	//convert to short int
	short int sample_int = (short int) strtol(sample_string, NULL, 16);

	//mutliply by factor and cast as long int 
	sample_int = (short int) sample_int * *curr_factor;

	//convert back to string
	char sample[100];
	sprintf(sample, "%04x", sample_int);

	//print the converted sample
	long int i = strlen(sample);
	
	printf("%c%c %c%c ", sample[i-2], sample[i-1], sample[i-4], sample[i-3]);

	return 0;
}


/*
FIN EFFECT
convert a single line of from stdin to the correct values. 

RETURN VALUES
- return 0 upon successful convertion
- return 1 upon EOF before convertion can happen
- return -1 upon error before convertion can happen
*/
int fin_convert_one_line(double factor, double *curr_factor, int channel) {
	
	//read offset and check for errors
	char offset[11];
	if (fgets(offset, 11, stdin) == NULL) {

		if (feof(stdin)) {
			return 1;
		}
		else if (ferror(stdin)) {
			return -1;
		}
	}
	printf("%s", offset);

	//read actual samples
	if (channel == 1) {
		for (int i = 0; i < 8; i++) {

			short int outcome = convert_one_sample(factor, curr_factor);
			if (outcome == 0) {
				if (*curr_factor < 1) {
					*curr_factor = *curr_factor + factor;
				}
			} 
			else {
				return outcome;
			}
		}
	}
	else {
		for (int i = 0; i < 4; i++) {

			short int outcome_b1 = convert_one_sample(factor, curr_factor);
			if (outcome_b1 == 0) {

				short int outcome_b2 = convert_one_sample(factor, curr_factor);
				if (outcome_b2 == 0) {
					if (*curr_factor < 1) {
						*curr_factor = *curr_factor + factor;
					}
				}
				else {
					return outcome_b2;
				}
			}
			else {
				return outcome_b1;
			}
		}
	}

	//read rest of the line (just garbage)
	char rest_of_line[19];
	if (fgets(rest_of_line, 19, stdin) == NULL) {

		if (feof(stdin)) {
			return 1;
		}
		else if (ferror(stdin)) {
			return -1;
		}
	}
	printf("%s", rest_of_line);
	return 0;
}


/*
FOUT EFFECT
convert a single line of from stdin to the correct values.

RETURN VALUES
- return 0 upon successful convertion
- return 1 upon EOF before convertion can happen
- return -1 upon error before convertion can happen
*/
int fout_convert_one_line(double factor, double *curr_factor2, long int *samples_that_dont_need_effect, int channel) {

	//read offset and check for errors
	char offset[11];
	if (fgets(offset, 11, stdin) == NULL) {

		if (feof(stdin)) {
			return 1;
		}
		else if (ferror(stdin)) {
			return -1;
		}
	}
	printf("%s", offset);

	//read actual samples
	short int outcome; 
	if (channel == 1) {
		for (int i = 0; i < 8; i++) {

			if (*samples_that_dont_need_effect <= 0) {

				outcome = convert_one_sample(factor, curr_factor2);
				if (outcome != 0) {
					return outcome;
				}
				*curr_factor2 = *curr_factor2 - factor;
			}

			else {

				double garbage_factor = 1.0;
				double garbage_curr_factor = 1.0;

				outcome = convert_one_sample(garbage_factor, &garbage_curr_factor);
				if (outcome != 0) {
					return outcome;
				}
				*samples_that_dont_need_effect = *samples_that_dont_need_effect - 1;
			}
		}
	}
	else {

		for (int i = 0; i < 4; i++) {

			if (*samples_that_dont_need_effect <= 0) {

				outcome = convert_one_sample(factor, curr_factor2);
				if (outcome != 0) {
					return outcome;
				}

				outcome = convert_one_sample(factor, curr_factor2);
				if (outcome != 0) {
					return outcome;
				}

				*curr_factor2 = *curr_factor2 - factor;
			}

			else {

				double garbage_factor = 1.0;
				double garbage_curr_factor = 1.0;
				
				outcome = convert_one_sample(garbage_factor, &garbage_curr_factor);
				if (outcome != 0) {
					return outcome;
				}

				outcome = convert_one_sample(garbage_factor, &garbage_curr_factor);
				if (outcome != 0) {
					return outcome;
				}

				*samples_that_dont_need_effect = *samples_that_dont_need_effect - 1;
			}
		}
	}
	

	//read rest of the line (just garbage)
	char rest_of_line[19];
	if (fgets(rest_of_line, 19, stdin) == NULL) {

		if (feof(stdin)) {
			return 1;
		}
		else if (ferror(stdin)) {
			return -1;
		}
	}
	printf("%s", rest_of_line);
	return 0;
}


/*
PAN EFFECT
convert a single line of from stdin to the correct values.

RETURN VALUES
- return 0 upon successful convertion
- return 1 upon EOF before convertion can happen
- return -1 upon error before convertion can happen
*/
int pan_convert_one_line(double factor, double *curr_factor, double *curr_factor2) {

	//read offset and check for errors
	char offset[11];
	if (fgets(offset, 11, stdin) == NULL) {

		if (feof(stdin)) {
			return 1;
		}
		else if (ferror(stdin)) {
			return -1;
		}
	}
	printf("%s", offset);

	//read actual samples
	for (int i = 0; i < 4; i++) {

		short int outcome_b1 = convert_one_sample(factor, curr_factor2);
		if (outcome_b1 == 0) {

			if (*curr_factor2 > 0) {
					*curr_factor2 = *curr_factor2 - factor;
			}

			short int outcome_b2 = convert_one_sample(factor, curr_factor);
			if (outcome_b2 == 0) {
				if (*curr_factor < 1) {
					*curr_factor = *curr_factor + factor;
				}
			}
			else {
				return outcome_b2;
			}
		}
		else {
			return outcome_b1;
		}
	}

	//read rest of the line (just garbage)
	char rest_of_line[19];
	if (fgets(rest_of_line, 19, stdin) == NULL) {

		if (feof(stdin)) {
			return 1;
		}
		else if (ferror(stdin)) {
			return -1;
		}
	}
	printf("%s", rest_of_line);
	return 0;
}


//END OF CODE