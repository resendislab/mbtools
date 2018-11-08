/*
 * Copyright 2018 Christian Diener <mail[at]cdiener.com>
 *
 * Apache License 2.0.
 */

#include <Rcpp.h>
#include <vector>
#include <numeric>
#include <sstream>

using namespace Rcpp;

unsigned int miniter = 20;

std::string vec_to_str(std::vector<int> vec) {
    std::stringstream ss;
    for (unsigned int i=0; i<vec.size(); ++i) {
        ss << "|" << vec[i];
    }
    return ss.str();
}

std::unordered_map<std::string, std::vector<int> > equivalence_classes(
    std::vector<std::vector<int> > reads_to_txs) {
    std::unordered_map<std::string, std::vector<int> > ecs;
    std::string ec;
    std::vector<int> txs;
    for (unsigned int i=0; i<reads_to_txs.size(); ++i) {
        txs = reads_to_txs[i];
        std::sort(txs.begin(), txs.end());
        ec = vec_to_str(txs);
        if (ecs.count(ec) > 0) {
            ecs[ec][0] += 1;
        } else {
            txs.insert(txs.begin(), 1);
            ecs[ec] = txs;
        }
    }

    return ecs;
}

// [[Rcpp::export]]
NumericVector effective_lengths(NumericVector txlengths,
                                NumericVector rdlengths) {
    NumericVector efflen(txlengths.size());
    NumericVector subset;
    double rdmean = mean(rdlengths);
    for (unsigned int i=0; i<txlengths.size(); ++i) {
        if (txlengths[i] >= rdmean) {
            efflen[i] = txlengths[i] - rdmean + 1;
        } else {
            subset = rdlengths[rdlengths <= txlengths[i]];
            efflen[i] = txlengths[i] - mean(subset) + 1;
        }
    }
    return round(efflen, 0);
}

// [[Rcpp::export]]
List em_count(NumericMatrix txreads, NumericVector txlengths,
                       int ntx, int nr, int maxit=1000, double cutoff=0.01) {
    std::vector<std::vector<int> > reads_to_txs(nr);
    NumericVector p(ntx, 1.0 / ntx);
    NumericVector pnew(ntx, 0.0);
    NumericVector txs, change;
    LogicalVector is_large;
    double read_sum, count;
    std::unordered_map<std::string, std::vector<int> > ecs;

    for (unsigned int i=0; i<txreads.nrow(); ++i) {
        if (txreads(i, 0) >= ntx) stop("wrong number of transcripts");
        if (txreads(i, 1) >= nr) stop("wrong number of reads");
        reads_to_txs[txreads(i, 1)].push_back(txreads(i, 0));
    }

    ecs = equivalence_classes(reads_to_txs);

    unsigned int k;
    for (k=0; k<maxit; ++k) {
        for (unsigned int i=0; i<ntx; ++i) {
            pnew[i] = 0.0;
        }
        for (std::pair<std::string, std::vector<int> > el : ecs) {
            read_sum = 0.0;
            txs = el.second;
            count = (double) txs[0];
            for (unsigned int txi=1; txi<txs.size(); ++txi) {
                read_sum += p[txs[txi]];
            }
            for (unsigned int txi=1; txi<txs.size(); ++txi) {
                pnew[txs[txi]] += count * p[txs[txi]] / read_sum;
            }
        }
        pnew = pnew / nr;
        pnew = (pnew / txlengths);
        pnew = pnew / sum(pnew);
        is_large = pnew > cutoff;
        change = abs(pnew - p) / pnew;
        change = change[is_large];
        if (max(change) < cutoff && k >= miniter) {
            break;
        }
        for (unsigned int i=0; i<ntx; ++i) p[i] = pnew[i];
    }

    return List::create(_["p"] = pnew,
                        _["iterations"] = k,
                        _["change"] = change,
                        _["num_ecs"] = ecs.size());
}