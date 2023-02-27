#' Title
#'
#' @param df
#'
#' @return
#' @export
#'
#' @examples
test <- function(df) {
  .Call("C_webservice", df, PACKAGE = "curlproject")
}
