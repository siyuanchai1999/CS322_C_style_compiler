int64 plus(tuple self, int64 x, int64 y)  {
  int64 ans
  ans <- x+y
  return ans
}

int64 minus(tuple self, int64 x, int64 y) {
  int64 ans
  ans <- x-y
  return ans
}

int64 times(tuple self, int64 x, int64 y) {
  int64 ans
  ans <- x*y
  return ans
}

void main()
{
:_start
  code plusptr
  code minusptr
  code timesptr
  tuple ops
  int64 ans

  ops <- new Tuple(3)
  plusptr <- plus
  minusptr <- minus
  timesptr <- times
  ops[0] <- plusptr
  ops[1] <- minusptr
  ops[2] <- timesptr

  int64 X
  int64 Y
  X <- 7
  Y <- 2

  int64 i
  int64 fin
  int64 num
  code f
  i <- 0
:L1
    f <- ops[i]
    ans <- f(ops, X, Y)
    num <- ans
    print(num)
    i <- i+1
    fin <- i < 3
  while (i < 3) :L1 :L2b

:L2b
  tuple cGs
  tuple cF
  tuple cG
  tuple cH
  cGs <- new Tuple(3)

  i <- 0
  while (1 = 1) :L2 :L3b
:L2
    f <- ops[i]
    cF <- new Tuple(3)
    cF[0] <- f
    cG <- curry(cF)
    cGs[i] <- cG
    i <- i+1
    if (i < 3) :L2 :L2e

:L2e
      break

:L3b
  code g
  code h
  i <- 0
  while (i < 3) :L3 :exit
:L3
  cG <- cGs[i]

  g <- cG[0]
  cH <- g(cG, X)

  h <- cH[0]
  ans <- h(cH, Y)

  num <- ans
  print(num)

  i <- i+1
  continue

:exit
  return
}

tuple curry(tuple F)
{
    tuple cF

    cF <- new Tuple(2)
    cF[0] <- :curry_arg1
    cF[1] <- F
    return cF
}

tuple curry_arg1(tuple self, int64 x)
{
    tuple cF1
    tuple F
    cF1 <- new Tuple(3)
    F <- self[1]
    cF1[0] <- :curry_arg2
    cF1[1] <- F
    cF1[2] <- x
    return cF1
}

int64 curry_arg2(tuple self, int64 y)
{
    tuple F
    int64 x
    code func
    int64 ans

    F <- self[1]
    x <- self[2]
    func <- F[0]
    ans <- func(F, x, y)
    return ans
}
