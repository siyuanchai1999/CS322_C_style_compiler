void main() {
  int64 M
  int64 N
  int64 res
  M <- 20000003
  N <- 1000000

  // trivial to optimize
  res <- compute_num(M)
  print(res)

  // ok to some degree
  static_arr(N)
  return
}


int64 val_float(int64 M) {
  int64 decimal
  decimal <- M * 37252902    // 37252902 = 10^16 / 2^28
  return decimal
}

// computes the slow-converging series pi := 4(1 - 1/3 + 1/5 - 1/7 + ...)
// using 28-bit fixed-point integer arithmetic
int64 compute_num(int64 N) {
  int64 sum
  sum <- 0

  int64 i
  i <- 0

  while (i < N) :add_elem :end {
:add_elem
    int64 b
    b <- 2*i
    b <- b+1

    int64 shift
    shift <- 0
    int64 quot
    quot <- b
    while (quot > 0) :do_log :go_div {
:do_log
      shift <- shift + 1
      quot <- quot >> 1
      continue
    }

:go_div
    int64 B
    B <- b << 28

    // compute 1 / b
    // reciprocal approximation via Newton's method
    // https://en.wikipedia.org/wiki/Multiplicative_inverse#Algorithms
    //
    // initial guess of the reciprocal
    int64 x
    x <- 1 << 28
    x <- x >> shift

    // using 40 iterations
    int64 j
    j <- 0
    {
:do_newton
      // x = (x * (2*M - ((B * x) >> L))) >> L
      // The (>> L) operation is part of fixed-point integer multiplication
      int64 B_x
      B_x <- B * x
      B_x <- B_x + 1    // rounding
      B_x <- B_x >> 28

      int64 twoM
      twoM <- 1 << 28
      twoM <- 2 * twoM

      int64 rhs
      rhs <- twoM - B_x

      x <- x * rhs
      x <- x + 1       // rounding
      x <- x >> 28

      j <- j + 1
    } while (j < 40) :do_newton :add

:add
    x <- x * 4
    int64 odd
    odd <- i & 1
    if (odd = 1) :neg :pos {
:neg
      sum <- sum - x
      goto :cont
    } :pos {
      sum <- sum + x
      goto :cont
    }
:cont
    i <- i + 1
    continue
  }
:end
  int64 ans
  ans <- val_float(sum)
  return ans
}

// basically the same as compute_num, but using an array
int64 static_arr(int64 N) {
  int64[][] newton
  int64[] inv
  int64 I
  int64 J

  // should never segfault
  // when N >= inv buffer size, array-error
  newton <- new Array( 1024, 16)
  int64 M
  M <- N - 100
  inv <- new Array(M)

  int64 i
  i <- 0

  int64 true
  true <- 0 < 1

  {
:comp
    if (i >= N) :end :add_elem {
:end
      return 0
      code unused
      break
    }
:add_elem
    int64 b
    b <- 2*i
    b <- b+1

    int64 shift
    shift <- 0
    int64 quot
    quot <- b
:log2
    while (quot > 0) :do_log :go_div {
:do_log
      shift <- shift + 1
      quot <- quot >> 1
      continue
    }

:go_div
    int64 B
    B <- b << 28

    int64 x
    x <- 1 << 28
    x <- x >> shift
    I <- i & 1023
    newton[I][0] <- x

    int64 j
    j <- 0
    while (j < 50) :do_newton :add {
:do_newton
      J <- j & 15
      x <- newton[I][J]
      int64 B_x
      B_x <- B * x
      B_x <- B_x + 1
      B_x <- B_x >> 28

      int64 twoM
      twoM <- 1 << 28
      twoM <- 2 * twoM

      int64 rhs
      rhs <- twoM - B_x

      x <- x * rhs
      x <- x + 1
      x <- x >> 28

      j <- j + 1
      J <- j & 15
      newton[I][J] <- x
      continue
    }

:add
    J <- j & 15
    x <- newton[I][J]
    x <- x * 4
    int64 odd
    odd <- i & 1
    if (odd = 1) :neg :pos {
:neg
      x <- 0 - x
    }
:pos
    inv[i] <- x
    int64 mask
    i <- i + 1
    mask <- i & 65535
    if (mask = 0) :debug :no_bug
:debug
      sum_and_print(inv, i)
:no_bug
  } while (true > 0) :comp :no
:no
  goto :no
}

void sum_and_print(int64[] arr, int64 N) {
  int64 i
  int64 sum
  int64 val

  i <- 0
  sum <- 0
  N <- N - 1
  {
:loop
    val <- arr[i]
    sum <- sum + val
    i <- i + 1
  } while (i < N) :loop :ok
:ok
  tuple buf
  buf <- new Tuple(3)
  val <- val_float(sum)
  buf[0] <- val
  val <- arr[N]
  sum <- sum + val
  val <- val_float(sum)
  buf[1] <- val
  N <- N + 1
  buf[2] <- N
  print(buf)
  return
}
