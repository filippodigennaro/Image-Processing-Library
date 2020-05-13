/*
 Created by Sebastiano Vascon on 23/03/20.
*/

#include <assert.h>
#include <stdio.h>
#include "ip_lib.h"
#include "bmp.h"

void ip_mat_show(ip_mat * t){
    unsigned int i,l,j;
    printf("Matrix of size %d x %d x %d (hxwxk)\n",t->h,t->w,t->k);
    for (l = 0; l < t->k; l++) {
        printf("Slice %d\n", l);
        for(i=0;i<t->h;i++) {
            for (j = 0; j < t->w; j++) {
                printf("%f ", get_val(t,i,j,l));
            }
            printf("\n");
        }
        printf("\n");
    }
}

void ip_mat_show_stats(ip_mat * t){
    unsigned int k;
    
    compute_stats(t);

    for(k=0;k<t->k;k++){
        printf("Channel %d:\n", k);
        printf("\t Min: %f\n", t->stat[k].min);
        printf("\t Max: %f\n", t->stat[k].max);
        printf("\t Mean: %f\n", t->stat[k].mean);
    }
}

ip_mat * bitmap_to_ip_mat(Bitmap * img){
    unsigned int i=0,j=0;

    unsigned char R,G,B;

    unsigned int h = img->h;
    unsigned int w = img->w;

    ip_mat * out = ip_mat_create(h, w,3,0);

    for (i = 0; i < h; i++)              /* rows */
    {
        for (j = 0; j < w; j++)          /* columns */
        {
            bm_get_pixel(img, j,i,&R, &G, &B);
            set_val(out,i,j,0,(float) R);
            set_val(out,i,j,1,(float) G);
            set_val(out,i,j,2,(float) B);
        }
    }

    return out;
}

Bitmap * ip_mat_to_bitmap(ip_mat * t){

    Bitmap *b = bm_create(t->w,t->h);

    unsigned int i, j;
    for (i = 0; i < t->h; i++)              /* rows */
    {
        for (j = 0; j < t->w; j++)          /* columns */
        {
            bm_set_pixel(b, j,i, (unsigned char) get_val(t,i,j,0),
                    (unsigned char) get_val(t,i,j,1),
                    (unsigned char) get_val(t,i,j,2));
        }
    }
    return b;
}

float get_val(ip_mat * a, unsigned int i,unsigned int j,unsigned int k){
    if(i<a->h && j<a->w &&k<a->k){  /* j>=0 and k>=0 and i>=0 is non sense*/
        return a->data[i][j][k];
    }else{
        printf("Errore get_val!!!");
        exit(1);
    }
}

void set_val(ip_mat * a, unsigned int i,unsigned int j,unsigned int k, float v){
    if(i<a->h && j<a->w &&k<a->k){
        a->data[i][j][k]=v;
    }else{
        printf("Errore set_val!!!");
        exit(1);
    }
}

float get_normal_random(){
    float y1 = ( (float)(rand()) + 1. )/( (float)(RAND_MAX) + 1. );
    float y2 = ( (float)(rand()) + 1. )/( (float)(RAND_MAX) + 1. );
    return cos(2*PI*y2)*sqrt(-2.*log(y1));

}

ip_mat * ip_mat_create(unsigned int h, unsigned int w,unsigned  int k, float v) {
    ip_mat *new_ip_mat;
    int i,j,p;

    new_ip_mat=(ip_mat *)malloc(sizeof(ip_mat));

    new_ip_mat->h=h;
    new_ip_mat->w=w;
    new_ip_mat->k=k;

    new_ip_mat->stat=(stats *)malloc(sizeof(stats)*k);
    for (i=0;i<k;i++) {
        new_ip_mat->stat[i].max=v;
        new_ip_mat->stat[i].min=v;
        new_ip_mat->stat[i].mean=v;
    }


    new_ip_mat->data=(float ***)malloc(sizeof(float **)*h);
    for (i=0;i<h;i++) {
        (new_ip_mat->data)[i]=(float **)malloc(sizeof(float*)*w);
        for (j=0;j<w;j++) {
            (new_ip_mat->data)[i][j]=(float *)malloc(sizeof(float)*k);
            for (p=0;p<k;p++) {
                (new_ip_mat->data)[i][j][p]=v;
            }
        }
    }
    return new_ip_mat;
}

void ip_mat_free(ip_mat *a){
    int i;
    int j;
    
    for(i=0;i<a->h;i++) {
        for(j=0;j<a->w;j++) {
            free(a->data[i][j]);
        }
        free(a->data[i]);
    }
    free(a->data);
    free(a->stat);
    free(a);
}

/*
 * Calcola il valore minimo, il massimo e la media per ogni canale
 * e li salva dentro la struttura ip_mat stats
 * */
void compute_stats(ip_mat * t) {
    int i,j,z;
    float count = 0.0;
    float max, min;
    float tot = 0;

    max = t->data[0][0][0];
    min = t->data[0][0][0];

    for (i=0;i<t->h;i++) {
        for (j=0;j<t->w;j++) {
            for (z=0;z<t->k;z++) {
                if (t->data[i][j][z] > max) 
                    max = t->data[i][j][z];

                if (t->data[i][j][z] < max) 
                    min = t->data[i][j][z];

                count++;

                tot += t->data[i][j][z];
            }
        }
    }

    tot = tot/(1.0 * count);

    t->stat->max = max;
    t->stat->min = min;
    t->stat->mean = tot;
}

/**** PARTE 1: OPERAZIONI MATEMATICHE FRA IP_MAT ****/
/* Esegue la somma di due ip_mat (tutte le dimensioni devono essere identiche)
 * e la restituisce in output. */
ip_mat * ip_mat_sum(ip_mat * a, ip_mat * b){
    ip_mat *new_ip_mat;
    int i,j,z;

    assert(a->w == b->w);
    assert(a->h == b->h);
    assert(a->k == b->k);

    new_ip_mat = ip_mat_create(a->w,a->h,a->k,0.0);

    if(a->h != b->h || a->w != b->w || a->k != b->k){
        printf("Error, ip_mat a and b are not the same size");
    }
    else{
        for(i=0;i<a->h;i++){
            for(j=0;j<a->w;j++){
                for(z=0;z<a->k;z++){
                    new_ip_mat->data[i][j][z] = a->data[i][j][z] + b->data[i][j][z];
                }
            }
        }
        compute_stats(new_ip_mat);
    }
    return new_ip_mat;
}

/* Esegue la sottrazione di due ip_mat (tutte le dimensioni devono essere identiche)
 * e la restituisce in output.
 * */
ip_mat * ip_mat_sub(ip_mat * a, ip_mat * b){
    ip_mat *new_ip_mat;
    int i,j,z;

    assert(a->w == b->w);
    assert(a->h == b->h);
    assert(a->k == b->k);

    new_ip_mat = ip_mat_create(a->w,a->h,a->k,0.0);

    if(a->h != b->h || a->w != b->w || a->k != b->k){
        printf("Error, ip_mat a and b are not the same size");
    }
    else{
        for(i=0;i<a->h;i++){
            for(j=0;j<a->w;j++){
                for(z=0;z<a->k;z++){
                    new_ip_mat->data[i][j][z] = a->data[i][j][z] - b->data[i][j][z];
                }
            }
        }
        compute_stats(new_ip_mat);
    }
    return new_ip_mat;
}

/* Moltiplica un ip_mat per uno scalare c. Si moltiplica c per tutti gli elementi di "a"
 * e si salva il risultato in un nuovo tensore in output. */
ip_mat * ip_mat_mul_scalar(ip_mat *a, float c){
        ip_mat *new_ip_mat;
    int i,j,z;
    new_ip_mat = ip_mat_create(a->w,a->h,a->k,0.0);

    for(i=0;i<a->h;i++){
        for(j=0;j<a->w;j++){
            for(z=0;z<a->k;z++){
                new_ip_mat->data[i][j][z] = a->data[i][j][z] * c;
            }
        }
    }
    compute_stats(new_ip_mat);
    return new_ip_mat;
}

/* Aggiunge ad un ip_mat uno scalare c e lo restituisce in un nuovo tensore in output. */
ip_mat *  ip_mat_add_scalar(ip_mat *a, float c){
    ip_mat *new_ip_mat = ip_mat_create(a->h,a->w,a->k,0.0);
    int i,j,z;
    for(i=0;i<a->h;i++){
        for(j=0;j<a->w;j++){
            for(z=0;z<a->k;z++){
                new_ip_mat->data[i][j][z] = a->data[i][j][z] + c;
            }
        }
    }
    compute_stats(new_ip_mat);
    return new_ip_mat;
}

/* Calcola la media di due ip_mat a e b e la restituisce in output.*/
ip_mat * ip_mat_mean(ip_mat * a, ip_mat * b){
    int i,j,z;

    assert(a->w == b->w);
    assert(a->h == b->h);
    assert(a->k == b->k);

    ip_mat *new_ip_mat = ip_mat_create(a->w,a->h,a->k,0.0);

    if(a->h != b->h || a->w != b->w || a->k != b->k){
        printf("Error, ip_mat a and b are not the same size");
    }
    else{
        for(i=0;i<a->h;i++){
            for(j=0;j<a->w;j++){
                for(z=0;z<a->k;z++){
                    new_ip_mat->data[i][j][z] = (a->data[i][j][z] + b->data[i][j][z])/2.0;
                }
            }
        }
        compute_stats(new_ip_mat);
    }
    return new_ip_mat;
}

void ip_mat_init_random(ip_mat * t, float mean, float var){
    int i, j, k;
    for (i=0;i<t->h; i++) {
        for (j=0;j<t->w;j++) {
            for (k=0;k<t->k;k++) {
                t->data[i][j][k]=(1/sqrt(2*PI*var*var)*exp(pow(get_normal_random()-mean, 2)/(2*var*var)));
            }
        }
    }
    compute_stats(t);
}

ip_mat * ip_mat_subset(ip_mat * t, unsigned int row_start, unsigned int row_end, unsigned int col_start, unsigned int col_end) {
    int i,j,k;

    ip_mat *new_ip_mat = ip_mat_create((row_end-row_start-1), (col_end-col_start-1), t->k, 0);

    for (i=row_start;i<row_end;i++) {
        for (j=col_start;j<col_end;j++) {
            for (k=0;k<new_ip_mat->k;k++) {
                new_ip_mat->data[i][j][k]=t->data[i][j][k];
            }
        }
    }
    compute_stats(new_ip_mat);

    return new_ip_mat;
}

ip_mat * ip_mat_copy(ip_mat * in){
    int i,j,z;

    ip_mat *new_ip_mat = ip_mat_create(in->w,in->h,in->k,0.0);

    new_ip_mat->stat = in->stat;

    for(i=0;i<in->h;i++){
        for(j=0;j<in->w;j++){
            for(z=0;z<in->k;z++){
                new_ip_mat->data[i][j][z] = in->data[i][j][z];
            }
        }
    }
    return new_ip_mat;
}

/*DONE*/
ip_mat * ip_mat_to_gray_scale(ip_mat * in) {
    int i, j, z;
    float mean;
    ip_mat * new_ip_mat =  ip_mat_create(in->h, in->w, in->k, 0.0);
    
    for(i=0; i<in->h; i++){
        for(j=0; j<in->w; j++){
            float tot = 0.0;
            for(z=0; z<in->k; z++){
                tot += in->data[i][j][z];
            }
            mean = tot/z;

            for(z=0; z<in->k; z++){
                new_ip_mat->data[i][j][z] = mean;
            }
        }
    }
    return new_ip_mat;
}


ip_mat * ip_mat_brighten(ip_mat * a, float bright){
    ip_mat *new_ip_mat = ip_mat_add_scalar(a,bright);
    return new_ip_mat;
}

/* FIXME: Da finire, non funziona */
ip_mat * ip_mat_corrupt(ip_mat * a, float amount){
    int i,j,z;
    ip_mat * out = ip_mat_create(a->h,a->w,a->k,0.0);
    for (i=0;i<a->h;i++) {
        for (j=0;j<a->w;j++) {
            for (z=0;z<a->k;z++) {
                out->data[i][j][z] = a->data[i][j][z] + i * amount;
            }
        }
    }
    compute_stats(out);
    return out;
}

/*DONE*/
ip_mat * ip_mat_blend(ip_mat * a, ip_mat * b, float alpha) {
    ip_mat *blend;
    int i, j, k;
    assert(a->h==b->h && a->w==b->w && a->k==b->k);
    blend=ip_mat_create(a->h, a->w, a->k, 0.);
    for (i=0;i<a->h;i++) {
        for (j=0;j<a->w;j++) {
            for (k=0;k<a->k;k++) {
                blend->data[i][j][k]=(alpha*a->data[i][j][k])+(1-alpha)*b->data[i][j][k];
            }
        }
    }
    compute_stats(blend);
    return blend;
}

/* Effettua la convoluzione di un ip_mat "a" con un ip_mat "f".
 * La funzione restituisce un ip_mat delle stesse dimensioni di "a".
 * */
ip_mat * ip_mat_convolve(ip_mat * a, ip_mat * f){
    int i, j, k;
    int x, y;
    int s;
    ip_mat * new_ip_mat = ip_mat_create(a->h,a->w,a->k,0.0);
    ip_mat * temp =(ip_mat_create(a->h+(f->h-1)/2, a->w+(f->w-1)/2, a->k, 0.));
    temp = ip_mat_padding(a,(f->h-1)/2,(f->w-1)/2);
    //Movimeto all'interno della matrice a
    for (i=0; i<(temp->h)-(f->h); i++){
        for (j=0; j<(temp->w)-(f->w); j++){
            for (k=0; k<temp->k; k++){
                //Somme nelle matrici
                // Scorre sul filtro
                s = 0.0;
                for (x=0; x<f->h; x++) { //vert
                    for (y=0; y<f->w; y++) { //horz
                        s += temp->data[i+x][j+y][k]*(f->data[x][y][0]);
                    }
                }
                new_ip_mat->data[i][j][k] = s;
            }
        }
    }
    compute_stats(new_ip_mat);
    return new_ip_mat;
}

/*DONE*/
ip_mat * ip_mat_padding(ip_mat * a, int pad_h, int pad_w){
    int i, j, k;
    ip_mat * new_ip_mat = ip_mat_create(a->h + 2*pad_h, a->w + 2*pad_w, a->k, 0.0);
    for (i=0; i<a->h; i++) {
        for (j=0; j<a->w; j++){
            for(k=0; k<a->k; k++){
                new_ip_mat->data[i+pad_h][j+pad_w][k] = a->data[i][j][k];
            }
        }
    }
    return new_ip_mat;
}

