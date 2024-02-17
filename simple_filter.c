#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>

#define FS 48000.0f
#define FL 1500.0f
#define FH 3500.0f
#define PI 3.141592653589793f
#define N_FFT 1200 // 傅立葉轉換點數
#define START 962880 // 20.060秒對應的取樣點
#define END 964080 // 20.085秒對應的取樣點

// 聲音波形的結構
typedef struct _wav {
	int fs;
	char header[44];
	size_t length;
	short *LChannel;
	short *RChannel;
} wav;

// 讀取wav檔案的函式
int wav_read_fn(char *fn, wav *p_wav)
{
	//char header[44];
	short temp = 0;
	size_t i = 0;

	FILE *fp = fopen(fn, "rb");
	if(fp==NULL) {
		fprintf(stderr, "cannot read %s\n", fn);
		return 0;
	}
	fread(p_wav->header, sizeof(char), 44, fp);
	while( !feof(fp) ) {
		fread(&temp, sizeof(short), 1, fp);
		i++;
	}
	p_wav->length = i / 2;
	p_wav->LChannel = (short *) calloc(p_wav->length, sizeof(short));
	if( p_wav->LChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for LChannel in wav_read_fn\n");
		fclose(fp);
		return 0;
	}
	p_wav->RChannel = (short *) calloc(p_wav->length, sizeof(short));
	if( p_wav->RChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for RChannel in wav_read_fn\n");
		fclose(fp);
		return 0;
	}
	fseek(fp, 44, SEEK_SET);
	for(i=0;i<p_wav->length;i++) {
		fread(p_wav->LChannel+i, sizeof(short), 1, fp);
		fread(p_wav->RChannel+i, sizeof(short), 1, fp);
	}
	fclose(fp);
	return 1;
}

// 儲存wav檔案的函式
int wav_save_fn(char *fn, wav *p_wav)
{
	FILE *fp = fopen(fn, "wb");
	size_t i;
	if(fp==NULL) {
		fprintf(stderr, "cannot save %s\n", fn);
		return 0;
	}
	fwrite(p_wav->header, sizeof(char), 44, fp);
	for(i=0;i<p_wav->length;i++) {
		fwrite(p_wav->LChannel+i, sizeof(short), 1, fp);
		fwrite(p_wav->RChannel+i, sizeof(short), 1, fp);
	}
	fclose(fp);
	return 1;
}

// 初始化wav結構的函式
int wav_init(size_t length, wav *p_wav)
{
	p_wav->length = length;
	p_wav->LChannel = (short *) calloc(p_wav->length, sizeof(short));
	if( p_wav->LChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for LChannel in wav_read_fn\n");
		return 0;
	}
	p_wav->RChannel = (short *) calloc(p_wav->length, sizeof(short));
	if( p_wav->RChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for RChannel in wav_read_fn\n");
		return 0;
	}
	return 1;
}

// 釋放wav結構所佔用的記憶體的函式
void wav_free(wav *p_wav)
{
	free(p_wav->LChannel);
	free(p_wav->RChannel);
}

/* hamming: for n=0,1,2,...N, length of N+1 */
float hamming(int N, int n) {
    return 0.54 - 0.46 * cosf(2 * PI * ((float)(n)) / ((float)N));
}

// 低通濾波器的頻率響應
float low_pass(int m, int n) {
    float wc = 2 * PI * FL / FS;
    if (n == m) {
        return wc / PI;
    } else {
        return sinf(wc * ((float)(n - m))) / PI / ((float)(n - m)) * hamming(2 * m + 1, n);
    }
}

// 高通濾波器的頻率響應
float high_pass(int m, int n) {
    float wc = 2 * PI * FH / FS;
    if (n == m) {
        return 1.0 - wc / PI;
    } else {
        return (sinf(PI * ((float)(n - m))) - sinf(wc * ((float)(n - m)))) / PI / ((float)(n - m)) * hamming(2 * m + 1, n);
    }
}

// 帶通濾波器的頻率響應
float band_pass(int m, int n)
{
    float wh = 2 * PI * FH / FS;
    float wl = 2 * PI * FL / FS;
    if (n == m) {
        return 2.0 * (wh / PI - wl / PI);
    }
    else {
        return 2.0 * (sinf(wh * ((float)(n - m))) - sinf(wl * ((float)(n - m)))) / PI / ((float)(n - m)) * hamming(2 * m + 1, n);
    }
}

// 傅立葉轉換函數
void DFT(short int *xn, int len, FILE *file_Y) {
    int a, n;
    float Xr[N_FFT];
    float Xi[N_FFT];

    for (a = 0; a < N_FFT; a++) {
        Xr[a] = 0;
        Xi[a] = 0;

        for (n = START; n <= END; n++) {
            Xr[a] += xn[n] * hamming(N_FFT - 1, n - START) * cosf(2 * PI * a * (n - START) / N_FFT);
            Xi[a] -= xn[n] * hamming(N_FFT - 1, n - START) * sinf(2 * PI * a * (n - START) / N_FFT);
        }

        // 取log後乘以20
        float magnitude = 20 * log10f(sqrtf(Xr[a] * Xr[a] + Xi[a] * Xi[a]));
        fprintf(file_Y, "%.15e\n", fabs(magnitude));
    }
}

int main(int argc, char **argv) {
    if (argc != 8) {
        fprintf(stderr, "Usage: %s M hL.txt hR.txt YL.txt YR.txt input.wav output.wav\n", argv[0]);
        return 1;
    }

    int M = atoi(argv[1]);
    char *hL_filename = argv[2];
    char *hR_filename = argv[3];
    char *YL_filename = argv[4];
    char *YR_filename = argv[5];
    char *input_filename = argv[6];
    char *output_filename = argv[7];


    // 檔案指標用於儲存濾波器係數和傅立葉轉換結果
    FILE *file_hL = fopen(hL_filename, "w");
    FILE *file_hR = fopen(hR_filename, "w");
    FILE *file_YL = fopen(YL_filename, "w");
    FILE *file_YR = fopen(YR_filename, "w");

    if (file_hL == NULL || file_hR == NULL || file_YL == NULL || file_YR == NULL) {
        fprintf(stderr, "Error opening files.\n");
        return 1;
    }

    wav wavin;
    wav wavout;
    float h_L[2 * M + 1];
    float h_R[2 * M + 1];
    int n = 0;
    float y = 0;
    int k;
    float yL = 0;
    float yR = 0;
    
     // 讀取wav檔案
    if (wav_read_fn(input_filename, &wavin) == 0) {
        fprintf(stderr, "cannot read wav file %s\n", input_filename);
        exit(1);
    }

     // 建立左聲道的帶通濾波器
    for (n = 0; n < (2 * M + 1); n++) {
        h_L[n] = band_pass(M, n);  // Amplify by 2
        fprintf(file_hL, "%.15e\n", h_L[n]);  // Write to hL.txt
        
    }

   // 建立右聲道的帶阻濾波器
    for (n = 0; n < (2 * M + 1); n++) {
        h_R[n] = high_pass(M, n) - low_pass(M, n);
        fprintf(file_hR, "%.15e\n", h_R[n]); 
    }

    // filtering (convolution)
    if (wav_init(wavin.length, &wavout) == 0) {
        exit(1);
    }

    for (n = 0; n < wavin.length; n++) {
        y = 0;
        for (k = 0; k < (2 * M + 1); k++) {
            if ((n - k) >= 0)
                y = y + h_L[k] * ((float)(wavin.LChannel[n - k]));
        }
        wavout.LChannel[n] = (short)(roundf(y));

        y = 0;
        for (k = 0; k < (2 * M + 1); k++) {
            if ((n - k) >= 0)
                y = y + h_R[k] * ((float)(wavin.RChannel[n - k]));
        }
        wavout.RChannel[n] = (short)(roundf(y));
    }
    memcpy(wavout.header, wavin.header, 44);

    // save wav
    if (wav_save_fn(output_filename, &wavout) == 0) {
        fprintf(stderr, "cannot save %s\n", output_filename);
        exit(1);
    }
    	// 進行傅立葉轉換並將結果寫入YL.txt和YR.txt檔案
    DFT(wavout.LChannel, wavout.length, file_YL); // 左聲道的傅立葉轉換
    DFT(wavout.RChannel, wavout.length, file_YR); // 右聲道的傅立葉轉換


    // Close the coefficient files
    fclose(file_hL);
    fclose(file_hR);
    wav_free(&wavin);
    wav_free(&wavout);
    fclose(file_YL);
    fclose(file_YR);
    printf("wav和.txt檔成功寫入\n");
    return 0;
}
