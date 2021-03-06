% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/context.R
\name{context}
\alias{context}
\alias{n}
\alias{cur_group}
\alias{cur_group_id}
\alias{cur_column}
\title{Context dependent expressions}
\usage{
n()

cur_group()

cur_group_id()

cur_column()
}
\description{
These functions return information about the "current" group or "current"
variable, so only work inside specific contexts like \code{summarise()} and
\code{mutate()}
\itemize{
\item \code{n()} gives the current group size.
\item \code{cur_group()} gives the group keys, a tibble with one row and one column
for each grouping variable.
\item \code{cur_group_id()} gives a unique numeric identifier for the current group.
\item \code{cur_column()} gives the current column (in \code{\link[=across]{across()}} only).
}
}
\examples{
df <- tibble(
  g = rep(letters[1:3], 1:3),
  x = runif(6),
  y = runif(6)
)
gf <- df \%>\% group_by(g)

gf \%>\% summarise(n = n())

gf \%>\% mutate(id = cur_group_id())
gf \%>\% mutate(across(everything(), ~ paste(cur_column(), round(.x, 2))))
}
