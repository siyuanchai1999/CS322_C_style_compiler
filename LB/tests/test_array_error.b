// test array error
// expected: 
// {s:15, 2, 3, 4, 11, 12, 13, 14, 21, 22, 23, 24, 31, 32, 33, 34}
// attempted to use position 4 (linearized array length: 15) 

void main () {
  int64[][] arr
  arr <- init(3, 4)
  print(arr)

  int64 v
  v <- arr[1][4] // out of range
  print(v) // should not print arr[2][0] == 31

  return
}

int64[][] init (int64 rows, int64 cols) {
  int64[][] ret
  ret <- new Array(rows, cols)
  int64 i, j, x
  i <- 0

  while (i < rows) :loop_out :end {
    :loop_out
    j <- 0
    while (j < cols) :loop_in :next {
      :loop_in
      x <- i * 10
      x <- x + j
      x <- x + 11
      ret[i][j] <- x
      j <- j + 1
      continue
    }
    :next
    i <- i + 1
    continue
  }
  :end
  return ret
}