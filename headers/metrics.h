#ifndef METRIC_H
#define METRIC_H

#include "image.h"

typedef struct {
    double start;
    double end;
    double processingTime;
    double speedup;
    double efficiency;
    int numberOfThreads;
} Metrics;

void calcProcessingTime(Metrics *metrics);
double calculateSpeedUp(Metrics *seq_metrics, Metrics *par_metrics);
double calculateEfficiency(Metrics *metrics);
void metrics_summary(Metrics *seq_metrics, Metrics *par_metrics);
void processOutput(Metrics *seq_metrics, Metrics *par_metrics);
#endif
