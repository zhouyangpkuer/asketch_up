#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <string.h>
#include <ctime>
#include <time.h>
#include <iterator>
#include <math.h>
#include <map>
#include <vector>
#include <algorithm>

#include "CMSketch.h"
#include "ASketch.h"
#include "ASketch_multi_filter.h"


#define testcycles 10


using namespace std;

uint insert[MAX_INSERT_PACKAGE + MAX_INSERT_PACKAGE / 500];
uint query[MAX_INSERT_PACKAGE + MAX_INSERT_PACKAGE / 500];

unordered_map<uint, int> unmp;
unordered_map<uint ,int> unmp_myres[2];
unordered_map<uint ,int> unmp_standres;

struct node {uint x; int y;} t[MAX_INSERT_PACKAGE / 5];

node t1[MAX_INSERT_PACKAGE / 5];

node t2[MAX_INSERT_PACKAGE / 5];

int CMP(node i,node j) {return i.y>j.y;}


int main(int argc, char** argv)
{
    char filename_FlowTraffic[500];
    // strcpy(filename_FlowTraffic, "../format_data/formatted06.dat");
    strcpy(filename_FlowTraffic, "./kosarak_form.dat");
    

    // strcpy(filename_FlowTraffic, "../format_data/webdocs_form.dat");
    // strcpy(filename_FlowTraffic, "../format_data/zipf1.8.dat");



    int package_num = 0;
    FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "rb");
    
    char *ip[8];
    while(fread(ip, 1, 8, file_FlowTraffic))
    {
        uint key = *((uint *)ip);
        insert[package_num] = key;

        unmp[key]++;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_FlowTraffic);

    printf("dataset: %s\n", filename_FlowTraffic);
    printf("total stream size = %d\n", package_num);
    printf("distinct item number = %d\n", unmp.size());

    int max_freq = 0;
    unordered_map<uint, int>::iterator it = unmp.begin();

    for(int i = 0; i < unmp.size(); i++, it++)
    {
        query[i] = it->first;
        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
    }
    printf("max_freq = %d\n\n", max_freq);




    double memory = 1.0;
    int d = 3;
    int filter_size = 128;

    if(argc == 2)
        filter_size = atoi(argv[1]); 

    CMSketch *cmsketch = new CMSketch(memory, d);
    ASketch *asketch = new ASketch(memory, d, filter_size);
    ASketch_mf *asketch_mf = new ASketch_mf(memory, d, filter_size);






















    
    timespec time1, time2;
    long long resns;
    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        cmsketch = new CMSketch(memory, d);
        for(int i = 0; i < package_num; i++)
        {
            cmsketch->Insert(insert[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cm = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of CM (insert): %.6lf\n", throughput_cm);
    

    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        asketch = new ASketch(memory, d, filter_size);
        for(int i = 0; i < package_num; i++)
        {
            asketch->Insert(insert[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_a = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of A (insert): %.6lf\n", throughput_a);



    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        asketch_mf = new ASketch_mf(memory, d, filter_size);
        for(int i = 0; i < package_num; i++)
        {
            asketch_mf->Insert(insert[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_a_mf = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of A_MF (insert): %.6lf\n\n", throughput_a_mf);





















    int flow_num = unmp.size();
    int temp = 0, sum = 0;

    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        for(int i = 0; i < package_num; i++)
        {
            temp += cmsketch->Query(insert[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cm_q = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of CM (query package): %.6lf\n", throughput_cm_q);
    sum += temp;
    

    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        for(int i = 0; i < package_num; i++)
        {
            temp += asketch->Query(insert[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_a_q = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of A (query package): %.6lf\n", throughput_a_q);
    sum += temp;
    

    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        for(int i = 0; i < package_num; i++)
        {
            temp += asketch_mf->Query(insert[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_a_q_mf = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of A_MF (query package): %.6lf\n\n", throughput_a_q_mf);
    sum += temp;
    






















    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            temp += cmsketch->Query(query[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cm_q_s = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of CM (query flow): %.6lf\n", throughput_cm_q_s);
    sum += temp;
    

    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            temp += asketch->Query(query[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_a_q_s = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of A (query flow): %.6lf\n", throughput_a_q_s);
    

    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            temp += asketch_mf->Query(query[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_a_q_s_mf = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of A_MF (query flow): %.6lf\n\n", throughput_a_q_s_mf);
    


    sum += temp;
    if(sum == (1 << 30))
        return 0;























    double AE1_package = 0.0, AE2_package = 0.0, AE3_package = 0.0;
    double AE1_flow = 0.0, AE2_flow = 0.0, AE3_flow = 0.0;

    int i = 0;

    FILE * res = fopen("result.txt", "w");
    for(int i = 0; i < package_num; i++)
    {
        uint key = insert[i];
        int val = unmp[key];

        int correct_cnt1 = cmsketch->Query(key);
        AE1_package += correct_cnt1 - val;


        int correct_cnt2 = asketch->Query(key);
        AE2_package += correct_cnt2 - val;

        if(correct_cnt2 < val)
            printf("a1 error\n");

        int correct_cnt3 = asketch_mf->Query(key);
        AE3_package += correct_cnt3 - val;

        if(correct_cnt3 < val)
            printf("a2 error\n");

        fprintf(res, "%d\t%d\t%d\t\n", val, correct_cnt1, correct_cnt2, correct_cnt3);
    }
    

    printf("AE1_package = %lf\n", AE1_package / package_num);
    printf("AE2_package = %lf\n", AE2_package / package_num);
    printf("AE3_package = %lf\n\n", AE3_package / package_num);

    






















    for(int i = 0; i < flow_num; i++)
    {
        uint key = query[i];
        int val = unmp[key];

        int correct_cnt1 = cmsketch->Query(key);
        AE1_flow += correct_cnt1 - val;


        int correct_cnt2 = asketch->Query(key);
        AE2_flow += correct_cnt2 - val;

        int correct_cnt3 = asketch_mf->Query(key);
        AE3_flow += correct_cnt3 - val;

    }

    printf("AE1_flow = %lf\n", AE1_flow / flow_num);
    printf("AE2_flow = %lf\n", AE2_flow / flow_num);
    printf("AE3_flow = %lf\n\n", AE3_flow / flow_num);













    int K = filter_size;
    int cnt = 0;
    for(unordered_map<uint, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        t[++cnt].x = it->first; 
        t[cnt].y = it->second;
    }
    sort(t + 1, t + cnt + 1, CMP);



    cnt = 0;
    uint * ID1 = asketch->get_items();
    int * count1 = asketch->get_freq();
    for(int i = 0; i < K; i++)
    {
        t1[++cnt].x = ID1[i]; 
        t1[cnt].y = count1[i];
    }
    sort(t1 + 1, t1 + cnt + 1, CMP);


    cnt = 0;
    uint * ID2 = asketch_mf->get_items();
    int * count2 = asketch_mf->get_freq();
    for(int i = 0; i < K; i++)
    {
        t2[++cnt].x = ID2[i]; 
        t2[cnt].y = count2[i];
    }
    sort(t2 + 1, t2 + cnt + 1, CMP);






    int correct_cnt1 = 0, correct_cnt2 = 0;
    int stand_val1 = 0, stand_val2 = 0;

    double RE1 = 0;
    double RE2 = 0;


    for (int i = 1; i <= K; i++)
    {
        uint X = t1[i].x;
        stand_val1 = unmp[X];

        for (int j = 1; j <= K; j++) 
        {
            if (X == t[j].x)
            {
                correct_cnt1++;
                RE1 += fabs(t1[i].y - stand_val1) * 1.0 / stand_val1;
            }
        }
    }


    for(int i = 1; i <= K; i++)
    {
        uint X = t2[i].x;
        stand_val2 = unmp[X];
     
        for (int j = 1; j <= K; j++)
        {
            if (X == t[j].x)
            {
                correct_cnt2++;
                RE2 += fabs(t2[i].y + stand_val2) * 1.0 / stand_val2;
            }
        }
    }


    double a = package_num * 1.0;
    double b = unmp.size() * 1.0;
    // fprintf(stderr, "SS1 precision rate %d/%d = %lf, recall rate %d/%d = %lf, with RE = %lf\n", correct_cnt1, H1, correct_cnt1 * 1.0 / H1, correct_cnt1, K, correct_cnt1 * 1.0 / K, RE1);
    // fprintf(stderr, "SS2 precision rate %d/%d = %lf, recall rate %d/%d = %lf, with RE = %lf\n", correct_cnt2, H2, correct_cnt2 * 1.0 / H2, correct_cnt2, K, correct_cnt2 * 1.0 / K, RE2);

    fprintf(stderr, "SS1 accepted %d/%d = %lf, with ARE = %lf\n", correct_cnt1, K, correct_cnt1 * 1.0 / K, RE1 * 1.0 / K);
    fprintf(stderr, "SS2 accepted %d/%d = %lf, with ARE = %lf\n\n", correct_cnt2, K, correct_cnt2 * 1.0 / K, RE2 * 1.0 / K);
    




    return 0;

}
