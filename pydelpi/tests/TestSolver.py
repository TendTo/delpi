import pytest

import pydelpi as pdi


@pytest.mark.parametrize("lp_solver_name", ([pdi.LpSolverName.QSOPTEX, pdi.LpSolverName.SOPLEX]))
class TestExpression:
    @pytest.fixture(scope='function', autouse=True)
    def setup_method(self, lp_solver_name: pdi.LpSolverName):
        self.x_ = pdi.Variable("x")
        self.y_ = pdi.Variable("y")
        self.z_ = pdi.Variable("z")
        self.e1_ = pdi.Expression()
        self.e2_ = pdi.Expression()
        config = pdi.Config(
            format=pdi.Format.MPS,
            lp_solver=lp_solver_name,
        )
        self.solver_ = pdi.LpSolver.get_instance(config)

    def test_add_column(self, lp_solver_name: pdi.LpSolverName):
        col_idx = self.solver_.add_column(self.x_)
        assert self.solver_.var(col_idx).equal_to(self.x_)
        assert self.solver_.column(col_idx).var.equal_to(self.x_)
        assert self.solver_.column(col_idx).lb == 0
        assert self.solver_.column(col_idx).ub is None
        assert self.solver_.column(col_idx).obj is None

    def test_add_column_objective(self, lp_solver_name: pdi.LpSolverName):
        col_idx = self.solver_.add_column(self.x_, 17)
        assert self.solver_.var(col_idx).equal_to(self.x_)
        assert self.solver_.column(col_idx).var.equal_to(self.x_)
        assert self.solver_.column(col_idx).lb == 0
        assert self.solver_.column(col_idx).ub is None
        assert self.solver_.column(col_idx).obj == 17

    def test_add_column_bounded(self, lp_solver_name: pdi.LpSolverName):
        col_idx = self.solver_.add_column(self.x_, 7, 8)
        assert self.solver_.var(col_idx).equal_to(self.x_)
        assert self.solver_.column(col_idx).var.equal_to(self.x_)
        assert self.solver_.column(col_idx).lb == 7
        assert self.solver_.column(col_idx).ub == 8
        assert self.solver_.column(col_idx).obj is None

    def test_add_column_complete(self, lp_solver_name: pdi.LpSolverName):
        col_idx = self.solver_.add_column(self.x_, 16, 8, 15)
        assert self.solver_.var(col_idx).equal_to(self.x_)
        assert self.solver_.column(col_idx).var.equal_to(self.x_)
        assert self.solver_.column(col_idx).lb == 8
        assert self.solver_.column(col_idx).ub == 15
        assert self.solver_.column(col_idx).obj == 16

    def test_add_row(self, lp_solver_name: pdi.LpSolverName):
        f = pdi.Formula(self.x_ + 2 * self.y_, pdi.FormulaKind.LEQ, 5)
        self.solver_.add_column(self.x_)
        self.solver_.add_column(self.y_)
        row_idx = self.solver_.add_row(f)
        assert len(self.solver_.row(row_idx).addends) == 2
        for var, coeff in self.solver_.row(row_idx).addends:
            assert f.expression.addends[var] == coeff
        assert self.solver_.row(row_idx).lb is None
        assert self.solver_.row(row_idx).ub == 5

    def test_optimise(self, lp_solver_name: pdi.LpSolverName):
        self.solver_.add_column(self.x_, 9)
        self.solver_.add_column(self.y_, 1)
        self.solver_.add_row(self.x_ + self.y_, pdi.FormulaKind.GEQ, 10)
        precision = 0
        result = self.solver_.solve(precision)
        assert result == pdi.LpResult.OPTIMAL
        assert self.solver_.solution(self.x_) == 0
        assert self.solver_.solution(self.y_) == 10
