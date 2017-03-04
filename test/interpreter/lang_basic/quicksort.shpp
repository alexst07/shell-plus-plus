func quicksort(A, lo, hi) {
  if (lo < hi) {
    p = partition(A, lo, hi)
    A = quicksort(A, lo, p - 1)
    A = quicksort(A, p + 1, hi)
  }

  return A
}

func partition(A, lo, hi) {
  pivot = A[hi]
  i = lo
  j = lo

  for j in range(lo, hi) {
    if A[j] <= pivot {
      A[i], A[j] = A[j], A[i]
      i = i + 1
    }
  }

  A[i], A[hi] = A[hi], A[i]
  return i
}

a = [1,4,6,4,10,11,22,100,3,3,9]
print(quicksort(a, 0, len(a) - 1))
