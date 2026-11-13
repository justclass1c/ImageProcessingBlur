#include "../headers/metrics.h"
#include <stdio.h>


void calcProcessingTime(Metrics *metrics) {
    double start = metrics -> start;
    double end = metrics -> end;

    metrics -> processingTime = end - start;
}

double calculateSpeedUp(Metrics *seq_metrics, Metrics *par_metrics) {
    if (par_metrics == NULL) return -1;

    if (seq_metrics -> processingTime == 0) par_metrics -> speedup = -1;
    else par_metrics -> speedup = seq_metrics -> processingTime / par_metrics -> processingTime;
    return par_metrics -> speedup;
}

double calculateEfficiency(Metrics *metrics) {
    if (metrics -> speedup <= 0 || metrics -> numberOfThreads <= 0) {
        metrics -> efficiency = -1;
        return metrics -> efficiency;
    }

    metrics -> efficiency = metrics -> speedup / metrics -> numberOfThreads;
    return metrics -> efficiency;
}

void metrics_summary(Metrics *seq_metrics, Metrics *par_metrics) {
    printf("Sequential Time, Ts = %f\n", seq_metrics -> processingTime);

    if (par_metrics == NULL) {
        printf("Parallel Time, Tp = N/A (Run a parallel blur first!)\n");
        printf("Speedup: N/A (Run a parallel blur first!)\n");
        printf("Efficiency: N/A (Run a parallel blur first!)\n");

        return;
    } 

    printf("Parallel Time, Tp = %f\n", par_metrics -> processingTime);

    if (par_metrics -> speedup <= 0) {
        printf("Speedup: N/A (Run a sequential blur first!)\n");
        printf("Efficiency: N/A (Run a sequential blur first!)\n");
    } else {
        printf("Speedup: %f\n", par_metrics -> speedup);
        printf("Efficiency: %f\n", par_metrics -> efficiency);
    } 
}

void processOutput(Metrics *seq_metrics, Metrics *par_metrics) {
    calcProcessingTime(seq_metrics);
    calcProcessingTime(par_metrics);
    calculateSpeedUp(seq_metrics, par_metrics);
    calculateEfficiency(par_metrics);
    metrics_summary(seq_metrics, par_metrics);
}
