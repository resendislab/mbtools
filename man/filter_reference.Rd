% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/remove_ref.R
\name{filter_reference}
\alias{filter_reference}
\title{Filter a set of reference sequences from the data set.seed}
\usage{
filter_reference(object, ...)
}
\arguments{
\item{object}{An experiment data table as returned by
\code{\link{find_read_files}} or a worflow object.}

\item{...}{A configuration as returned by
\code{\link{config_reference}}.}
}
\value{
A list with the processed files and removal counts for each sample.
}
\description{
This will also return a data.table containing the counts of reference
sequences for each sample.
}
