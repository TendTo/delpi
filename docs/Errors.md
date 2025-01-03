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

Furthermore, a subscripted letter "$^T$" will indicate the transpose of the corresponding matrix or vector and a subscripted "$^{-1}$" will indicate the inverse of the corresponding matrix.

## Floating-point arithmetic model

When it comes to dealing with real numbers, the limited available memory and the desire for a fast computation forces computers to approximate their representation. Almost [all modern devices operate](https://nhigham.com/accuracy-and-stability-of-numerical-algorithms-bib-entry/) the [IEEE 754 standard](https://ieeexplore.ieee.org/document/8766229) implementing floating-point arithmetic. It is designed in such a way that the evaluation of an expression, denoted as $\bold{fl}(\cdot)$, where the operator $\circ = +, -, -, \star$, satisfies

$$
\bold{fl}_\epsilon(x \circ y) = (x \circ y)(1 + z) \quad |z| \le \epsilon
$$

where $\epsilon$ the the **unit roundoff** (or **machine epsilon**), which represents the smallest possible value the system can distinguish from $0$ and it is inversely exponential in the number of bits of precision. It is usually in the order of $10^{-8}$ or $10^{-16}$ in single and double precision computer arithmetic. The same notation can be extended to include arbitrary linear algebra expressions within $\bold{fl}_\epsilon(\cdot)$ and it is to be understood as that each scalar operation underlying the expression is to be carried out in that same precision, in an arbitrary order.

### Types of error

Suppose to calculate an approximation $\hat{y}$ of $y = f(x)$ using some floating-point arithmetic with precision $\epsilon$.
To measure the quality of the results, meaning how close $\hat{y}$ is to $y$, we may consider different definitions.
A very common one is asking how close the computed result is to the exact one, i.e. the **forward error**, but more often we are interested in how close the computed result is to the best possible result, i.e. the **backward error**.

<center>
  <img src="_static/img/stability.svg" alt="Forward vs Backward propagation" width="500" class="img-svg"/>
</center>
<center>
  <div>Fig. 1 <i>Backward and forward errors for $y = f(x)$. Solid line = exact; dotted line = computed. </i></div> 
</center>

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
