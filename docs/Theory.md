# Theory

Theory behind the solver.
While its knowledge is not necessary to use the solver, it can be useful to understand how it works, especially when debugging or extending its features.

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

<center>
  <img src="_static/img/stability.svg" alt="Forward vs Backward propagation" width="500" class="img-svg"/>
</center>
<center>
  <div>Fig. 1 <i>Backward and forward errors for $y = f(x)$. Solid line = exact; dotted line = computed. </i></div> 
</center>

## Linear Programming

A **LP** problem is an optimisation problem where the objective function and the constraints are linear equalities or inequalities.
The objective function is what we want to maximise or minimise, while the constraints are the conditions the solution must satisfy.
The standard form of a **LP** problem is the following:

$$
\begin{equation*}
  \begin{aligned}
    & \max              & \boldsymbol{c}^T \boldsymbol{x}      \newline
    & \text{subject to} & \boldsymbol{A} \boldsymbol{x} \leq \boldsymbol{b} \newline
    &                   & \boldsymbol{x} \geq 0
  \end{aligned}
\end{equation*}
$$

where $\boldsymbol{x} \in \mathbb{R}^d$ is the vector of variables to be determined, $\boldsymbol{c} \in \mathbb{R}^d$ and $\boldsymbol{b} \in \mathbb{R}^n$ are vectors of coefficients, and $\boldsymbol{A} \in \mathbb{R}^{n \times d}$ is a matrix of coefficients.
It is always possible to rewrite a **LP** problem in standard form following these steps:

- If the problem is a minimization problem, it is sufficient to multiply the objective function by $-1$ to obtain the corresponding maximization problem.
- If some variables have no lower bound, they can be substituted with the difference of two variables, both with a lower bound of $0$ (i.e. $x = x_1 - x_2$ and $x_1, x_2 \geq 0$).
- If strict equalities exist, they can be substituted with two inequalities (i.e. $x_i = 0$ becomes $x_i \leq 0$ and $x_i \geq 0$).
- If there are some inequalities with different signs, one can be multiplied by $-1$ to ensure both have the same sign.

**LP** problems are usually solved via the simplex method, although some [interior-point methods](https://en.wikipedia.org/wiki/Interior-point_method) may be used as well.
The [simplex method](https://en.wikipedia.org/wiki/Simplex_algorithm) is an iterative algorithm developed by George Dantzig in 1947.
To apply the simplex method, the problem must be converted in _slack form_ to use the simplex method.
Starting with a **LP** problem in standard form, the slack form is obtained by introducing a slack variable for each constraint so that the inequality becomes an equality.
The slack variables must also be non-negative.

$$
\begin{equation*}
  \begin{aligned}
    & \max              & \boldsymbol{c}^T \boldsymbol{x}      \newline
    & \text{subject to} & \boldsymbol{A} \boldsymbol{x} + \boldsymbol{s} = \boldsymbol{b} \newline
    &                   & \boldsymbol{x} \geq 0 \newline
    &                   & \boldsymbol{s} \ge 0
  \end{aligned}
\end{equation*}
$$

It is possible to extract an invertible squared matrix $B$ from $A$ of dimension $n \times n$ called basis.
The remaining columns of $A$ form the $N$ matrix.

$$
\begin{equation*}
  \boldsymbol{A} = \begin{bmatrix}
    \boldsymbol{B} & \boldsymbol{N}
  \end{bmatrix}
  \text{, where }
  \begin{cases}
    \boldsymbol{B} \in \mathbb{R}^{n \times n} \newline
    \boldsymbol{N} \in \mathbb{R}^{n \times (d - n)}
  \end{cases}
\end{equation*}
$$

The variables $x_i$ whose index $i$ corresponds to a column of $A$ also belonging to $B$ are called _basic variables_, while the ones that end up in $N$ _non-basic variables_.
For commodity, we can also define $I_B, I_N$ to be the set of indexes of such basic and non-basic variables respectively.
An example is shown below.

$$
\boldsymbol{A} = \begin{bmatrix}
  a_{1,1} & a_{1,2} & a_{1,3} & a_{1,4} \newline
  a_{2,1} & a_{2,2} & a_{2,3} & a_{2,4} \newline
  a_{3,1} & a_{3,2} & a_{3,3} & a_{3,4}
\end{bmatrix} \newline
\boldsymbol{B} = \begin{bmatrix}
  a_{1,1} & a_{1,2} & a_{1,4} \newline
  a_{2,1} & a_{2,2} & a_{2,4} \newline
  a_{3,1} & a_{3,2} & a_{3,4}
\end{bmatrix} \quad
\boldsymbol{N} = \begin{bmatrix}
  a_{1,3} \newline
  a_{2,3} \newline
  a_{3,3}
\end{bmatrix} \newline
I_B = \{1, 2, 4\} \quad I_N = \{3\}
$$

For commodity, we may also use the map $\nu : \{1, 2, \dots, m\} \to I_B$ to indicate the indexes of other matrices or vectors with respect to the $\boldsymbol{B}$ matrix.
Reusing the example above, we might write:

$$
\boldsymbol{B} = \begin{bmatrix}
    \boldsymbol{A}_{\nu_1} & \boldsymbol{A}_{\nu_2} & \boldsymbol{A}_{\nu_2}
\end{bmatrix} \quad
I_B = \{1, 2, 4\} \newline
\text{where } \boldsymbol{A}_i \text{ is the } i \text{th column vector of } \boldsymbol{A} \newline
\nu_1, \nu_2, \nu_3 \in I_B \quad
\nu_1 = 1, \nu_2 = 2, \nu_3 = 4
$$

If $\boldsymbol{x}$ is a **feasible solution**, meaning it satisfies all the constraints, and has $\boldsymbol{x}_j = 0 \quad \forall j \in I_N$ then it is called a **basic feasible solution** and all the columns of $\boldsymbol{A}$ present in $\boldsymbol{B}$ are called **basic vectors** or simply **basis** associated with $\boldsymbol{x}$. If $x_i = 0$ for some $i \in I_B$, then $\boldsymbol{x}$ is a **degenerate basic feasible solution**.

The simplex algorithms starts from a basic feasible solution $x$ and iterates producing a new basic feasible solution $\boldsymbol{x}'$ such that $\boldsymbol{c}^T \boldsymbol{x}' \lt \boldsymbol{c}^T \boldsymbol{x}$.
Furthermore, if $\boldsymbol{x}'$ is optimal, the algorithm terminates indicating it.
The same happens if the objective function is unbounded.
In the worst possible case the algorithm will visit all possible basis before reaching a conclusion, which means ${n \choose m} = \frac{n!}{m!(n - m)!}$ iterations.
Despite this, the simplex tends to perform quite well in practice, with the number of cycle employed usually being close to a small multiple of $m$.

Reaching a degenerate solutions may cause problems in the algorithm, causing its properties not to hold anymore.
However, there are standard ways of adjusting a given linear programming problem so avoid such scenarios.

### Solving the LP

The technique used to produce the optimal solution $\hat{x}$ utilises basic matrices.
Let $\boldsymbol{B} = \begin{bmatrix}\boldsymbol{A}_{\nu_i}& \dots & \boldsymbol{A}_{\nu_m}\end{bmatrix}$ be our basic matrix, $\boldsymbol{x}_B = \begin{bmatrix}x_{\nu_i}& \dots & x_{\nu_m}\end{bmatrix}^T$ be the vector of non-zero components of $\boldsymbol{x}$ and $\boldsymbol{x}_N$ be the vector containing all the components of $\boldsymbol{x}$ not in $\boldsymbol{x}_B$.
A similar reasoning is applied to $\boldsymbol{c}_B, \boldsymbol{c}_N$, which are the components of the $\boldsymbol{c}$ vector corresponding to the basic and non-basic variables respectively.
We know that the value of the objective function is equal to $z = \boldsymbol{c}^T_B\boldsymbol{x}_B + \boldsymbol{c}^T_N\boldsymbol{x}_N$.
We want to represent everything in terms of the $\boldsymbol{x}_N$ elements to easily understand how the value will change once if we set the to something with is not $0$.
We know that for the solution to be feasible, the following constraints must hold:

$$
\boldsymbol{B}\boldsymbol{x}_B + \boldsymbol{N}\boldsymbol{x}_N = \boldsymbol{b} \newline
\boldsymbol{x}_B = \boldsymbol{B}^{-1}(\boldsymbol{b} - \boldsymbol{N}\boldsymbol{x}_N)
$$

then, the objective value can be rewritten as

$$
z = \boldsymbol{c}^T_B\boldsymbol{x}_B + \boldsymbol{c}^T_N\boldsymbol{x}_N \newline
z = \boldsymbol{c}^T_B\boldsymbol{B}^{-1}(\boldsymbol{b} - \boldsymbol{N}\boldsymbol{x}_N) + \boldsymbol{c}^T_N\boldsymbol{x}_N \newline
z = \boldsymbol{c}^T_B\boldsymbol{B}^{-1}\boldsymbol{b} + (\boldsymbol{c}_N^T - \boldsymbol{c}^T_B\boldsymbol{B}^{-1}\boldsymbol{N})\boldsymbol{x}_N
$$

Note that the first term is fixed once we have picked a base, but we try to improve the objective value by increasing the value of one of the components in the $x_N$ vector, which are now all set to $0$, provided its corresponding coefficient, $(\boldsymbol{c}^T_N - \boldsymbol{c}^T\boldsymbol{B}^{-1}\boldsymbol{N})$, is negative.

With all of this in mind, we can go through the following steps:

1. Solve $\boldsymbol{B}\boldsymbol{x}_B = \boldsymbol{b}$
2. Solve $\boldsymbol{B}^T\boldsymbol{\pi} = \boldsymbol{c}_B$.
   - Note that this means that $\boldsymbol{\pi} = \boldsymbol{B}^{-T}\boldsymbol{c}_B = (\boldsymbol{c}_B^T \boldsymbol{B}^{-1})^T$. Hence, $\boldsymbol{\pi} \in \mathbb{R}^{m}$. This way we don't need to compute $\boldsymbol{B}^{-1}$
3. For each column vector $\boldsymbol{A}_j$ of $\boldsymbol{A}$ not in the basis ($j \in I_N$) compute $\bar{c}_j = c_j - \boldsymbol{\pi}^T\boldsymbol{A}_j$ and choose any index $s$ where $\bar{c}_s > 0$
   - Usually the largest possible positive value $\bar{c}_s$ is chosen, hoping that would lead to faster convergence
4. Solve $\boldsymbol{B}\boldsymbol{y} = \boldsymbol{A}_s$
   - Note that this means that $\boldsymbol{y} = B^{-1}A_s$, which are the coefficients by which the non-basic variable $x_s$ will be multiplied by. We must ensure that, when this value is subtracted by the current value of all the current basic variables, none of them becomes negative and at least one becomes $0$
5. Let $r$ be the index such that $\frac{{x_\nu}_r}{y_r} = \min_{y_i \gt 0} \frac{{x_B}_i}{y_i}$
   - In other words, find the element of $\boldsymbol{x_B}$ that creates the minimum positive ratio with $\boldsymbol{y}$ and use its index to determine the column to remove from the basis
6. Drop the column $A_{\nu_r}$ from the basis $\boldsymbol{B}$ and replace it with the column vector $A_s$

If we can't find a positive value in step $3$, we can conclude that the current solution is optimal, since there is no way of improving it further.
Likewise, if step $5$ does not produce any result, meaning all ratios are negative, we can terminate knowing that the problem is unbounded.

#### Example

We are given the following LP problem

$$
\begin{equation*}
  \begin{aligned}
      & \min              & -x_1 - 2x_2 + x_3  \newline
      & \text{subject to} & 2x_1 + x_2 + x_3 \le 14 \newline
      &                   & 4x_1 + 2x_2 + 3x_3 \le 28 \newline
      &                   & 2x_1 + 5x_2 + 5x_3 \le 30 \newline
      &                   & x_1, x_2, x_3 \geq 0
  \end{aligned}
\end{equation*}
$$

The first step is to convert it to its slack form:

$$
\begin{equation*}
  \begin{aligned}
      & \max              & x_1 + 2x_2 - x_3  \newline
      & \text{subject to} & 2x_1 + x_2 + x_3 + s_1 = 14 \newline
      &                   & 4x_1 + 2x_2 + 3x_3 + s_2 = 28 \newline
      &                   & 2x_1 + 5x_2 + 5x_3 + s_3 = 30 \newline
      &                   & x_1, x_2, x_3, s_1, s_2, s_3 \geq 0
  \end{aligned}
\end{equation*}
$$

We use the slack variable to populate the initial base.
So we can build the following vectors:

$$
\boldsymbol{c}^T_B =\begin{bmatrix}
  0 & 0 & 0
\end{bmatrix} \quad
\boldsymbol{c}^T_N =\begin{bmatrix}
  1 & 2 & -1
\end{bmatrix} \quad
\boldsymbol{b}^T =\begin{bmatrix}
  14 & 28 & 30
\end{bmatrix} \newline
\boldsymbol{A} = \begin{bmatrix}
  2 & 1 & 1 & 1 & 0 & 0 \newline
  4 & 2 & 3 & 0 & 1 & 0 \newline
  2 & 5 & 5 & 0 & 0 & 1
\end{bmatrix} \newline
\boldsymbol{B} = \begin{bmatrix}
  1 & 0 & 0 \newline
  0 & 1 & 0  \newline
  0 & 0 & 1
\end{bmatrix} \quad
\boldsymbol{N} = \begin{bmatrix}
  2 & 1 & 1 \newline
  4 & 2 & 3  \newline
  2 & 5 & 5
\end{bmatrix}
$$

We can now proceed with the algorithm.

$$
\boldsymbol{B}^T\boldsymbol{x}_B = \boldsymbol{b} \Rightarrow \boldsymbol{x}_B = \begin{bmatrix} 14 & 28 & 30 \end{bmatrix}^T \newline
\boldsymbol{B}^T\boldsymbol{\pi} = \boldsymbol{c}_B \Rightarrow \boldsymbol{\pi} = \begin{bmatrix} 0 & 0 & 0 \end{bmatrix}^T \newline
s = \mathop{\mathrm{arg\,max}}_{\bar{c}_s > 0} (\bar{\boldsymbol{c}}^T = \boldsymbol{c}^T_N - \boldsymbol{\pi}^T\boldsymbol{N}) = \mathop{\mathrm{arg\,max}}_{\bar{c}_s > 0} (\begin{bmatrix} 1 & 2 & -1 \end{bmatrix}^T) = 2 \newline
\boldsymbol{B}\boldsymbol{y} = \boldsymbol{A}_s \Rightarrow \boldsymbol{y} = \begin{bmatrix} 1, 2, 5 \end{bmatrix}^T \newline
r = \mathop{\mathrm{arg\,min}}_{d_r > 0} \left(\frac{\boldsymbol{x}_B}{\boldsymbol{y}} \right) = 3
$$

Having computed both $s, r$, we have now the indexes of the entering and exiting variables respectively.
Therefore, the basis $B$ gets updated, as well as all depending matrices:

$$
\boldsymbol{c}^T_B =\begin{bmatrix}
  0 & 0 & 2
\end{bmatrix} \quad
\boldsymbol{c}^T_N =\begin{bmatrix}
  1 & 0 & -1
\end{bmatrix} \newline
\boldsymbol{B} = \begin{bmatrix}
  1 & 0 & 1 \newline
  0 & 1 & 2  \newline
  0 & 0 & 5
\end{bmatrix} \quad
\boldsymbol{N} = \begin{bmatrix}
  2 & 0 & 1 \newline
  4 & 0 & 3  \newline
  2 & 1 & 5
\end{bmatrix}
$$

We go back and perform another iteration of the algorithm.

$$
\boldsymbol{B}^T\boldsymbol{x}_B = \boldsymbol{b} \Rightarrow \boldsymbol{x}_B = \begin{bmatrix} 8 & 16 & 6 \end{bmatrix}^T \newline
\boldsymbol{B}^T\boldsymbol{\pi} = \boldsymbol{c}_B \Rightarrow \boldsymbol{\pi} = \begin{bmatrix} 0 & 0 & 0.4 \end{bmatrix}^T \newline
s = \mathop{\mathrm{arg\,max}}_{\bar{c}_s > 0} (\bar{\boldsymbol{c}}^T = \boldsymbol{c}^T_N - \boldsymbol{\pi}^T\boldsymbol{N}) = \mathop{\mathrm{arg\,max}}_{\bar{c}_s > 0} (\begin{bmatrix} 0.2 & -0.4 & -3 \end{bmatrix}^T) = 1 \newline
\boldsymbol{B}\boldsymbol{y} = \boldsymbol{A}_s \Rightarrow \boldsymbol{y} = \begin{bmatrix} 1.6 & 3.2 & 0.4 \end{bmatrix}^T \newline
r = \mathop{\mathrm{arg\,min}}_{d_r > 0} \left(\frac{\boldsymbol{x}_B}{\boldsymbol{y}} \right) = 1
$$

Having computed $s, r$ again, we can update $B$ and all depending matrices:

$$
\boldsymbol{c}^T_B =\begin{bmatrix}
  1 & 0 & 2
\end{bmatrix} \quad
\boldsymbol{c}^T_N =\begin{bmatrix}
  0 & 0 & -1
\end{bmatrix} \newline
\boldsymbol{B} = \begin{bmatrix}
  2 & 0 & 1 \newline
  4 & 1 & 2  \newline
  2 & 0 & 5
\end{bmatrix} \quad
\boldsymbol{N} = \begin{bmatrix}
  1 & 0 & 1 \newline
  0 & 0 & 3  \newline
  0 & 1 & 5
\end{bmatrix}
$$

And go through another iteration:

$$
\boldsymbol{B}^T\boldsymbol{x}_B = \boldsymbol{b} \Rightarrow \boldsymbol{x}_B = \begin{bmatrix} 5 & 0 & 4 \end{bmatrix}^T \newline
\boldsymbol{B}^T\boldsymbol{\pi} = \boldsymbol{c}_B \Rightarrow \boldsymbol{\pi} = \begin{bmatrix} 0.125 & 0 & 0.375 \end{bmatrix}^T \newline
s = \mathop{\mathrm{arg\,max}}_{\bar{c}_s > 0} (\bar{\boldsymbol{c}}^T = \boldsymbol{c}^T_N - \boldsymbol{\pi}^T\boldsymbol{N}) = \mathop{\mathrm{arg\,max}}_{\bar{c}_s > 0} (\begin{bmatrix} -0.125 & -0.375 & -3 \end{bmatrix}^T) = \bot
$$

This time we are not able to find an index $s$ that satisfies the conditions.
It means that we have no way of improving the objective value further.
Hence, the solution is optimal.

### Simplex implementation

The outlined algorithm needs to solve three linear systems in order to produce the final result. Those are also the most expensive operations.

$$
\boldsymbol{B}\boldsymbol{x}_B = \boldsymbol{b} \newline
\boldsymbol{B}^T\boldsymbol{\pi} = \boldsymbol{c}_b \newline
\boldsymbol{B}\boldsymbol{y} = \boldsymbol{A}_s \newline
$$

If we consider them in a vacuum, the computational complexity will be in the order of $O(m^3)$. Luckily we can be more efficient by exploiting the fact that the basic matrix does not change much at each iteration (only one column is replaced with another). The same can be said about $\boldsymbol{c}_B$, where only one component changes, and $\boldsymbol{b}$, which is constant through all iterations.

### Using LU decomposition

Instead of trying to solve all the systems directly, we can opt to use an **LU** decomposition of $\boldsymbol{B}$ to
improve the performance of the algorithm. Solving the linear systems in this setup becomes much easier, requiring only
two back-propagation steps each. The great challenge is to ensure that the decomposition, which has a complexity
of $O(m^3)$, is updated cheaply at each iteration, without having to recompute it each time.

The process starts by computing the LU decomposition of $\boldsymbol{B}$ in the standard way:

$$
\boldsymbol{B} = \boldsymbol{L}\boldsymbol{U} \newline
\boldsymbol{L} = \begin{bmatrix}
  l_{1,1} & & & & &  \newline
  l_{2,1} & l_{2,2} & & &  &  \newline
  \vdots & & \ddots &  &  &  \newline
  l_{m-1,1} & l_{m - 1,2} & \dots & l_{m - 1,m - 1} &  \newline
  l_{m,1} & l_{m,2} & \dots & l_{m,m-1} & l_{m,m}
\end{bmatrix} \quad
\boldsymbol{U} = \begin{bmatrix}
  u_{1,1} & u_{2,2} & \dots & u_{1,m-1} & u_{1,m} \newline
   		   & u_{2,2} & \dots & u_{2,m-1} & u_{2,m}  \newline
  & & \ddots &  & \vdots  \newline
  & & & u_{m - 1,m - 1} & u_{m - 1,m} \newline
  & & & & u_{m,m}
\end{bmatrix} \quad
$$

where $\boldsymbol{L} \in \mathbb{R}^{m \times m}$ is a lower triangular matrix and $\boldsymbol{U} \in \mathbb{R}^{m \times m}$ is an upper triangular matrix.

Suppose we now need to update the $\boldsymbol{B}$, replacing its $r$-th column $\boldsymbol{A}_{\nu_r}$ with $\boldsymbol{A}_s$, which we put at the end of the matrix. We can write this updated matrix $\boldsymbol{B}^{(1)}$ as

$$
\boldsymbol{B}^{(1)} = \begin{bmatrix}
\newline
\boldsymbol{B}_{1 \dots r - 1} & \boldsymbol{B}_{r + 1 \dots m} & A_s \newline
\newline
\end{bmatrix}
$$

where $\boldsymbol{B}_{i \dots j}$ are column vectors of $\boldsymbol{B}$ from column $i$ to column $j$. We would like to keep $\boldsymbol{L}$ fixed, only updating the upper triangular matrix to account for the change in columns. We obtain the the equation $\boldsymbol{B}^{(1)} = \boldsymbol{L}\boldsymbol{H}^{(1)}$, where

$$
\boldsymbol{H}^{(1)} = \begin{bmatrix}
U_{1\dots r-1} & U_{r+1 \dots m} & L^{-1}A_s
\end{bmatrix}
$$

Note that, due to the nature of matrix multiplication, we can express the rearrangement of the columns of $\boldsymbol{B}$ by doing the same to the columns of $\boldsymbol{U}$, needing to compute only the newly introduced column.
In almost all cases the new matrix $\boldsymbol{H}^{(1)}$ will not be triangular, since it the column permutation has introduced a "bump" with non-zero in the first sub-diagonal, making it an [upper Hessenberg matrix](https://en.wikipedia.org/wiki/Hessenberg_matrix).

<center>
  <img src="_static/img/H.svg" alt="H^(1)" width="500" class="img-svg"/>
</center>
<center>
  <div>Fig. 2 <i>Upper Hessenberg matrix $\boldsymbol{H}^{(1)}$.<br>The indexes, where present, refer to the columns of $\boldsymbol{U}$</i></div> 
</center>
Since we want to work with triangular matrices, we need to compute some elimination transformation matrix $\boldsymbol{E}^{(1)}$ that applied to $\boldsymbol{H}^{(1)}$ produces an upper triangular matrix $\boldsymbol{U}^{(1)} = \boldsymbol{E}^{(1)}\boldsymbol{H}^{(1)}$.
Hence, we obtain the equation $\boldsymbol{B}^{(1)} = \boldsymbol{L} \boldsymbol{E}^{(1)^{-1}} \boldsymbol{U}^{(1)} = \boldsymbol{L} \boldsymbol{E}^{(1)^{-1}}\boldsymbol{E}^{(1)}\boldsymbol{H}^{(1)}$.
The same process can be repeated for iteratively $k$ times, constructing a new $\boldsymbol{H}^{(k)}$ from $\boldsymbol{U}^{(k-1)}$ and $\boldsymbol{L}\boldsymbol{E}^{(1)^{-1}}\boldsymbol{E}^{(2)^{-1}}\dots\boldsymbol{E}^{(k - 1)^{-1}}$, applying $\boldsymbol{E}^{(k)}$ to it to compute $\boldsymbol{U}^{(k)}$ and thus obtaining 
$$
\boldsymbol{B}^{(k)} =  \boldsymbol{L}\boldsymbol{E}^{(1)^{-1}}\boldsymbol{E}^{(2)^{-1}}\dots\boldsymbol{E}^{(k-1)^{-1}}\boldsymbol{E}^{(k)^{-1}}\boldsymbol{U}^{(k)}
$$
How to choose $\boldsymbol{E}$ and how to store and handle the resulting data structures is what makes different implementation more or less performing with respect to the desired metric (e.g. stability, speed, memory consumption).

### LU for Stability

The [Bartels-Golub method](https://doi.org/10.1007/BF02169151) takes the LU decomposition approach with numerical stability in mind.
Generalising to the $k$-th iteration, it achieves the re-triangularisation of $\boldsymbol{H}^{(k)}$ by applying a series of simple permutation matrices $\boldsymbol{\Pi}_i$, which are equal to the identity matrix possibly having exactly two rows exchanged, and trivial lower triangular matrices $\boldsymbol{M}_i$, which are equal to the identity matrix except for one non-zero element outside the main diagonal, $\forall i \in \{r, r+1, \dots, m-2, m - 1\}$.
As a result

$$
\boldsymbol{U}^{(k)} = \boldsymbol{M}^{(k)}_{m-1}\boldsymbol{\Pi}^{(k)}_{m-1} \boldsymbol{M}^{(k)}_{m - 2}\boldsymbol{\Pi}^{(k)}_{m - 2} \dots \boldsymbol{M}^{(k)}_{r}\boldsymbol{\Pi}^{(k)}_{r} \boldsymbol{H}^{(k)}
$$

Note that the index $i$ ranges from $r$ to $m − 1$ because $r$ is the column of $\boldsymbol{B}^{(k)}$ that was removed during the last iteration and $m − 1$ is the penultimate column of $\boldsymbol{B}^{(k)}$. The last column, $m$, needs no adjustment because it is entirely in the upper triangular part of the matrix.

To simplify the notation, let

$$
\boldsymbol{C}^{(k)} = \boldsymbol{\Pi}^{(k)^{-1}}_{r} \boldsymbol{M}^{(k)^{-1}}_{r} \dots \boldsymbol{\Pi}^{(k)^{-1}}_{m-2} \boldsymbol{M}^{(k)^{-1}}_{m-2} \boldsymbol{\Pi}^{(k)^{-1}}_{m-1} \boldsymbol{M}^{(k)^{-1}}_{m-1}
$$

which is to say that $\boldsymbol{C}^{(k)}$ cancels out all the previous transformations.
Hence we obtain

$$
\boldsymbol{B}^{(k)} = \boldsymbol{L}\boldsymbol{C}^{(1)}\boldsymbol{C}^{(2)}\dots\boldsymbol{C}^{(k)}\boldsymbol{U}^{(k)}
$$

By defining

$$
\boldsymbol{G}^{(k)} = \boldsymbol{C}^{(1)}\boldsymbol{C}^{(2)}\dots\boldsymbol{C}^{(k)}
$$

We can make for expression even more compact:

$$
\boldsymbol{B}^{(k)} = \boldsymbol{L}\boldsymbol{G}^{(k)}\boldsymbol{U}^{(k)}
$$

Putting everything together, to solve a generic linear system

$$
\boldsymbol{B}^{(k)} \boldsymbol{v} = \boldsymbol{q} \quad (\text{or, similarly, } \boldsymbol{B}^{(k)^T} \boldsymbol{v} = \boldsymbol{q})
$$

with this setup we can perform a backpropagation solve with the lower triangular matrix $\boldsymbol{L}$

$$
\boldsymbol{L}\boldsymbol{t} = \boldsymbol{q}
$$

applying the transformations

$$
\boldsymbol{w} = [\boldsymbol{M}^{(k)}_{m-1}\boldsymbol{\Pi}^{(k)}_{m-1} \dots \boldsymbol{M}^{(k)}_{r}\boldsymbol{\Pi}^{(k)}_{r} \boldsymbol{H}^{(k)}] \dots [\boldsymbol{M}^{(1)}_{m-1}\boldsymbol{\Pi}^{(1)}_{m-1} \dots \boldsymbol{M}^{(1)}_{r}\boldsymbol{\Pi}^{(1)}_{r} \boldsymbol{H}^{(1)}] \boldsymbol{t}
$$

and finally solving

$$
\boldsymbol{U}^{(k)}\boldsymbol{v} = \boldsymbol{w}
$$

#### Error analysis in the Bartels-Golub method

Let us examine more in detail the error introduced using floating point arithmetic during the execution of the Bartels-Golub simplex algorithm.
Consider the expression

$$
\boldsymbol{B}^{(k)} + \mathbb{e}\boldsymbol{B}^{(k)} = \boldsymbol{L}\boldsymbol{G}^{(k)}\boldsymbol{U}^{(k)}
$$

where $\mathbb{e}\boldsymbol{B}^{(k)}$ indicates the error as the difference between the computed $\boldsymbol{B}^{(k)}$ and its true value that would be obtained with an exact computation of $\boldsymbol{L}\boldsymbol{G}^{(k)}\boldsymbol{U}^{(k)}$.

### Tableau

With this framework, it is possible to define a tableau.
Each row of the tableau corresponds to a constraint.
An additional row called _objective row_, is added to represent the objective function associated with a special variable $z$.
All the original variables are considered _non-basic_, while the slack variables are _basic_.

$$
\begin{equation*}
\begin{array}{c}
\newline
z \newline
s*1 \newline
s_2 \newline
s_3
\end{array}
\begin{bmatrix}
\begin{array}{c|cccccc|c}
z & x_1 & x_2 & x_3 & s_1 & s_2 & s_3 & b \newline \hline
1 & -c_1 & -c_2 & -c_3 & 0 & 0 & 0 & 0 \newline \hline
0 & a*{11} & a*{12} & a*{13} & 1 & 0 & 0 & b*1 \newline
0 & a*{21} & a*{22} & a*{23} & 0 & 1 & 0 & b*2 \newline
0 & a*{31} & a*{23} & a*{33} & 0 & 0 & 1 & b_3 \newline
\end{array}
\end{bmatrix}
\end{equation*}
$$

To iterate the simplex method, the following steps are performed:

- Find the pivot column $j$, which is the column with the most negative coefficient in the objective row.
- Find the pivot row $i$, which is the row with the smallest ratio between the constant term $b_i$ and the coefficient of the pivot column.
- Divide the pivot row by the coefficient of the pivot column so that the value in position $(i, j)$ becomes $1$.
- All other rows must contain the value $0$ along the pivot column. This is achieved by subtracting the pivot row multiplied by the coefficient of the pivot column.
- The variable corresponding to the pivot column becomes basic, while the one corresponding to the pivot row on the left of the tableau becomes non-basic.
- Repeat until the objective row has no negative coefficients.

The solution is obtained by reading the corresponding value from the tableau's last column, while the objective function's value can be found in the top right corner.

### Farkas' lemma

To prove that an **LP** problem is feasible, it is sufficient to find a vector $x$ and verify that $Ax \le b$.
It is possible to find a similar proof for the infeasibility of a problem.

The _Farkas' lemma_ states that only one of the following two statements is true for a given pair $(A, b)$:

$$
\begin{array}{l}
\exists x : Ax \le b \newline
\newline
\exists y : y^T A \le 0 \land y^T b > 0
\end{array}
$$

For **LP** problems, the vector $y$ is obtained by solving the dual problem.
Consider the following **LP** problem:

$$
\begin{equation*}
\begin{aligned}
& \max & c^T x \newline
& \text{subject to} & A x \geq b \newline
& & x \geq 0
\end{aligned}
\end{equation*}
$$

The corresponding dual problem is:

$$
\begin{equation*}
\begin{aligned}
& \min & b^T y \newline
& \text{subject to} & A^T y \le c \newline
& & y \geq 0
\end{aligned}
\end{equation*}
$$

By the _weak duality theorem_, it is possible to use the extreme ray from the unbounded dual problem to construct the vector $y$, called **Farkas' Ray**, that satisfies Farkas' lemma.

## References

- [Introduction to Operations Research](https://www.amazon.com/Introduction-Operations-Research-10th/dp/0134444019) by Hillier and Lieberman
- [Accuracy and Stability of Numerical Algorithms](https://www.amazon.com/Accuracy-Stability-Numerical-Algorithms-Second/dp/0898715210) by Nicholas J. Higham
- [Matrix Computations](https://www.amazon.com/Matrix-Computations-Johns-Hopkins-Studies/dp/1421407949) by Gene H. Golub and Charles F. Van Loan
