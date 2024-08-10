import numpy as np
import matrix_multiply

A = np.array([1.0, 2.0, 3.0], dtype=np.float32)
B = np.array([4.0, 5.0, 6.0], dtype=np.float32)

result = matrix_multiply.multiply_matrices(A, B)
print(result)  # Element-wise product of A and B