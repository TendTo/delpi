# Errors

Theory behind errors in floating point arithmetic used by the solver.
While its knowledge is not necessary to use the software, it can be useful to understand how it works, especially when debugging or extending its features.

## Legend

To ensure an easier reading, we will use the following notation:

- bold capital letters $\boldsymbol{A}, \boldsymbol{B}, \boldsymbol{C}$ for matrices
- subscripted bold capital letters $\boldsymbol{A}_i, \boldsymbol{B}_i, \boldsymbol{C}_i$ for column vectors
- subscripted bold capital letters $\boldsymbol{A}_{i \dots j}, \boldsymbol{B}_{i \dots j}, \boldsymbol{C}_{i \dots j}$ for submatrices going from column $i$ to column $j$
- bold lowercase letters $\boldsymbol{c}, \boldsymbol{x}, \boldsymbol{y}, \boldsymbol{z}$ for vectors
- subscripted bold lowercase letters $\boldsymbol{c}_i, \boldsymbol{x}_i, \boldsymbol{y}_i, \boldsymbol{z}_i$ for components of vectors
- non-bold lowercase letters $a, b, c, x, y, z$ for scalars
- absolute value of bold capital (lowercase)
  letters $|\boldsymbol{A}|, |\boldsymbol{B}|, (|\boldsymbol{x}|, |\boldsymbol{y}|)$ for matrices (vector) with
  constructed from the component-wise application of the absolute value operator

Furthermore, a subscripted letter "$^T$" will indicate the transpose of the corresponding matrix or vector and a subscripted "$^{-1}$" will indicate the inverse of the corresponding matrix.

## Floating-point arithmetic model

When it comes to dealing with real numbers, the limited available memory and the desire for a fast computation forces computers to approximate their representation. Almost [all modern devices operate](https://nhigham.com/accuracy-and-stability-of-numerical-algorithms-bib-entry/) the [IEEE 754 standard](https://ieeexplore.ieee.org/document/8766229) implementing floating-point arithmetic. It is designed in such a way that the evaluation of an expression, denoted as $\bold{fl}(\cdot)$, where the operator $\circ = +, -, -, \star$, satisfies

$$
\bold{fl}_\epsilon(x \circ y) = (x \circ y)(1 + \delta) \quad |z| \le \epsilon
$$

where $\epsilon$ the the **unit roundoff** (or **machine epsilon**), which represents the smallest possible value the system can distinguish from $0$ and it is inversely exponential in the number of bits of precision. It is usually in the order of $10^{-8}$ or $10^{-16}$ in single and double precision computer arithmetic. The same notation can be extended to include arbitrary linear algebra expressions within $\bold{fl}_\epsilon(\cdot)$ and it is to be understood as that each scalar operation underlying the expression is to be carried out in that same precision, in an arbitrary order.

## Types of error

Suppose to calculate an approximation $\hat{y}$ of $y = f(x)$ using some floating-point arithmetic with precision $\epsilon$.
To measure the quality of the results, meaning how close $\hat{y}$ is to $y$, we may consider different definitions.  
The most immediate approach is to consider the difference between the exact and the computed value, which is called the
**forward error** and is defined as $\Delta y = |\hat{y} - y|$.
Most of the times it is convenient to use th **backward error** definition, where we are interested in determining the
smallest $\Delta x$ such that $\hat{y} = f(x + \Delta x)$.
A algorithm is said to be **backward stable** if the backward error is "small" for all inputs, with "small" being
problem-dependent, but hopefully in the same order of magnitude as $\epsilon$.
Note that the definition of floating-point arithmetic given in the previous section is backward stable for $\circ = \pm$
and perturbed data $x(1 + \delta), y(1 + \delta)$.

$$
\bold{fl}_\epsilon(x\pm y) = x(1 + \delta) \pm y(1 + \delta) = x \pm y + \delta(x \pm y) = (x \pm y)(1 + \delta)\quad |\delta| \le \epsilon
$$

<center>
  <img src="_static/img/stability.svg" alt="Forward vs Backward propagation" width="500" class="img-svg"/>
</center>
<center>
  <div>Fig. 1 <i>Backward and forward errors for $y = f(x)$. Solid line = exact; dotted line = computed. </i></div> 
</center>

### Conditioning

The **condition number** of a problem is a measure of how sensitive the solution is to perturbations in the data.
The general definition is given by

$$
\mathrm{cond}(f,x) = \lim_{\varepsilon \to 0} \sup_{\|\Delta x\| \le \varepsilon \|x\|} \displaystyle\frac{\|f(x+\Delta x) - f(x)\|}{\varepsilon\|f(x)\|}
$$

There is a useful relationship between the condition number, the forward error and the backward error, given by

$$
\text{forward error} \lesssim \text{condition number} \times \text{backward error}
$$

## Inner and outer products

Consider the inner product $s$ of two vectors $\boldsymbol{x}, \boldsymbol{y} \in \mathbb{R}^n$ such
that $s = \boldsymbol{x}^T \boldsymbol{y}$.
We know that the value of $s$ is given by

$$
s = \sum_{i=1}^n x_i y_i
$$

and if we indicate with $s_k$ the partial sum of the first $k$ terms, we can write

$$
s_k = \sum_{i=1}^k x_i y_i
$$

Using the standard floating-point arithmetic model, we can write the computed value of $s_n$ and assuming all the
perturbation for each operation to be equal to a single $\delta : |\delta| \le \epsilon$, we can write

$$
\begin{array}{ll}
\hat{s}_1 &= \bold{fl}(x_1 y_1) = x_1 y_1 (1 + \delta) \newline
\hat{s}_2 &= \bold{fl}(\hat{s}_1 + x_2 y_2) = \hat{s}_1 + x_2 y_2 (1 + \delta) =\newline
&= ((x_1 y_1)(1 + \delta) + (x_2 y_2)(1 + \delta))(1 + \delta) = (x_1y_1)(1 + \delta)^2 + (x_2y_2)(1 + \delta)^2 \newline
\hat{s}_3 &= \bold{fl}(\hat{s}_2 + x_3 y_3) = \hat{s}_2 + x_3 y_3 (1 + \delta) = \newline
&= ((x_1 y_1)(1 + \delta)^2 + (x_2 y_2)(1 + \delta)^2 + (x_3 y_3)(1 + \delta))(1 + \delta) = \newline
&= (x_1y_1)(1 + \delta)^3 + (x_2y_2)(1 + \delta)^3 + (x_3y_3)(1 + \delta)^2
\end{array}
$$

In the general case, the patter is

$$
\hat{s}_n = x_1y_1(1 + \delta)^n + x_2y_2(1 + \delta) + x_3y_3(1 + \delta)^{n-1} + \dots + x_ny_n(1 + \delta)^2
$$

Assuming that all $|\delta_i| \le \epsilon$ and $n \epsilon \lt 1$, we can simplify the long expression introducing the
notation $\Theta_n$ and $\gamma_n$ as

$$
\prod_{i=1}^n (1 + \delta_i) = 1 + \Theta_n \newline
|\Theta_n| \le \frac{n\epsilon}{ 1 - n\epsilon} = \gamma_n
$$

<details>
<summary>Proof by induction</summary>

For $n = 1$ we have

$$
1 + \delta_1 \le 1 + \epsilon
$$

which is true for by the assumption $|\delta_1| \le \epsilon$.  
Assuming the statement to be true for $n$, we have

$$
\prod_{i=1}^{n+1} (1 + \delta_i) = (1 + \Theta_n)(1 + \delta_{n+1}) = 1 + \underbrace{\Theta_n + \delta_{n+1} + \Theta_n \delta_{n+1}}_{\Theta_{n+1}} \newline
\Theta_{n+1} = \delta_{n+1} + (1 + \delta_{n + 1})\Theta_n \newline
|\Theta_{n+1}| \le \epsilon + (1 + \epsilon)|\Theta_n| \le \epsilon + (1 + \epsilon)\frac{n\epsilon}{1 - n\epsilon} = \frac{(n+1)\epsilon}{1 - n\epsilon} \le \frac{(n+1)\epsilon}{1 - (n+1)\epsilon}
$$

</details>

## Triangular systems

Triangular systems are extremely important in numerical linear algebra, as they are very commonly used in the solution
of linear systems.
Hence, they have been studied extensively and their properties are well known.
They are extremely stable when it comes to backward errors.
The same cannot be said for forward errors, although in practice they tend to perform surprisingly well.

### Backward error analysis

Let $\boldsymbol{U} \in \mathbb{R}^{n \times n}$ be an upper triangular matrix and $\boldsymbol{b}$ a vector.
The system $\boldsymbol{U} \boldsymbol{x} = \boldsymbol{b}$ can be solved by back substitution using the formula

$$
\boldsymbol{x}_i = \frac{\boldsymbol{b}_i - \sum_{j=i+1}^n \boldsymbol{U}_{ij} \boldsymbol{x}_j}{\boldsymbol{U}_{ii}}
$$

where $\boldsymbol{U}_{i,i+1\dots n}$ indicates the $i$-th row $\boldsymbol{U}$ from column $i+1$ to $n$.

which yields the components of the solution vector $\boldsymbol{x}$ from the last to the first.
A simple implementation may look like

```python
def back_substitution(U, b):
    n = len(b)
    x = np.zeros(n)
    for i in range(n - 1, -1, -1):
        x[i] = (b[i] - U[i, i + 1 :] @ x[i + 1 :]) / U[i, i]
    return x
```

**Cost**: $n^2$ flops.

If $\hat{\boldsymbol{x}}$ is the computed solution, the backward error is defined as

$$
(\boldsymbol{U} + \Delta \boldsymbol{U}) \hat{\boldsymbol{x}} = \boldsymbol{b} \quad | \Delta \boldsymbol{U} | \le \gamma_n |\boldsymbol{U}|
$$

This result ensures that $\boldsymbol{x}$ has a tiny component-wise relative backward error.
In other words, the backward error is about as small as we could
possibly hope.
Furthermore, the same results hold for a lower triangular matrix $\boldsymbol{L}$ and the
system $\boldsymbol{L} \boldsymbol{x} = \boldsymbol{b}$.

<details>
<summary>Proof TODO</summary>

TODO

</details>

## LU factorization

The LU factorization of a matrix $\boldsymbol{A} \in \mathbb{R}^{n \times n}$ is a factorization of the
form $\boldsymbol{A} = \boldsymbol{LU}$, where $\boldsymbol{L}$ is a lower triangular matrix with unit diagonal
and $\boldsymbol{U}$ is an upper triangular matrix.

Implementing the LU factorization is a bit more complex than the back substitution, but it is still a relatively simple
algorithm.
There are multiple ways to implement it, but the simplest to analyse is the Doolittle factorization, which is defined as

```python
def lu_factorization_dolittle(A):
    n = A.shape[0]
    U = np.zeros((n, n))
    L = np.eye(n)
    for k in range(n):
        for j in range(k, n):
            U[k, j] = A[k, j] - L[k, :k] @ U[:k, j]
        for i in range(k + 1, n):
            L[i, k] = (A[i, k] - L[i, :k] @ U[:k, k]) / U[k, k]
    return L, U
```

**Cost**: $n^2(m - \frac{n}{3})$ flops.

### Backward error analysis

Noticing how the LU factorization employs two inner products in its iterations, we can use the previous results to
determine the backward error of the computed factorization.

$$
\hat{\boldsymbol{L}} \hat{\boldsymbol{U}} = \boldsymbol{A} + \Delta \boldsymbol{A} \quad | \Delta \boldsymbol{A} | \le \gamma_n |\hat{\boldsymbol{L}}||\hat{\boldsymbol{U}} |
$$

Extending the process, a similar result to linear systems solved with triangular matrices can be obtained.

$$
(A + \Delta A) \hat{x} = b \quad | \Delta A | \le \gamma_{3n} |\hat{\boldsymbol{L}}||\hat{\boldsymbol{U}} |
$$

## References

- [Accuracy and Stability of Numerical Algorithms](https://nhigham.com/accuracy-and-stability-of-numerical-algorithms-bib-entry/)
  ```bib
    @book{high:ASNA2,
    author    = {Nicholas J. Higham},
    title     = {Accuracy and Stability of Numerical Algorithms},
    publisher = {Society for Industrial and Applied Mathematics},
    address   = {Philadelphia, PA, USA},
    year      = 2002,
    edition   = {Second},
    pages     = {xxx+680},
    isbn      = {0-89871-521-0}
    }
  ```
