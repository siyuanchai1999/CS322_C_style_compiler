// This program tests the "code" type feature
// (and LB's variable scope binding)
// expected output:
// 1
// 2
// 12

int64 double (int64 v) {
  v <- v * 2
  return v
}

int64 plus_10 (int64 v) {
  v <- v + 10
  return v
}

void main () {
  int64 v
  code func

  v <- 1
  print(v)

  func <- plus_10
  { // add something misleading:
    code func
    func <- double
    v <- func(v)
    print(v)
  }
  v <- func(v)
  print(v)

  return
}