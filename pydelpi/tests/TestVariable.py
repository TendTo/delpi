import re

import pydelpi as pdi


class TestVariable:
    def setup_method(self):
        self.d_ = pdi.Variable()
        self.x_ = pdi.Variable("x")
        self.y_ = pdi.Variable("y")
        self.z_ = pdi.Variable("z")

    def test_dummy_constructor(self):
        d = pdi.Variable()
        assert d.id == pdi.Variable.dummy_id
        assert d.name == "dummy"
        assert d.is_dummy
        assert str(d) == "dummy"
        assert re.match(r"<Variable 'dummy' at 0x[a-f0-9]+>", repr(d))

    def test_variable_constructor(self):
        a = pdi.Variable("a")
        assert 0 <= a.id < pdi.Variable.dummy_id
        assert a.name == "a"
        assert str(a) == "a"
        assert re.match(r"<Variable 'a' at 0x[a-f0-9]+>", repr(a))

    def test_variable_equality(self):
        assert not self.d_.equal_to(self.x_)
        assert not self.x_.equal_to(self.d_)
        assert self.x_.equal_to(self.x_)

        assert not self.x_.equal_to(self.y_)
        assert not self.x_.equal_to(self.z_)
        assert not self.y_.equal_to(self.z_)
        assert self.x_.equal_to(self.x_)
        assert self.d_.equal_to(pdi.Variable())

    def test_variable_hash(self):
        assert hash(self.d_) != hash(self.x_)
        assert hash(self.x_) != hash(self.y_)
        assert hash(self.y_) != hash(self.z_)
        assert hash(self.d_) == hash(self.d_)
        assert hash(self.x_) == hash(self.x_)
        assert hash(self.y_) == hash(self.y_)
        assert hash(self.z_) == hash(self.z_)

    def test_variable_less(self):
        assert self.x_.less(self.d_)
        assert self.y_.less(self.d_)
        assert self.z_.less(self.d_)
        assert self.x_.less(self.y_)
        assert not self.y_.less(self.x_)
        assert self.y_.less(self.z_)
        assert not self.z_.less(self.y_)
        assert not self.x_.less(self.x_)
        assert not self.y_.less(self.y_)
        assert not self.z_.less(self.z_)
        new_var = pdi.Variable("new_var")
        assert self.x_.less(new_var)
        assert self.y_.less(new_var)
        assert self.z_.less(new_var)
        assert not self.d_.less(new_var)

    def test_variable_ostream(self):
        assert str(self.d_) == "dummy"
        assert str(self.x_) == "x"
        assert str(self.y_) == "y"
        assert str(self.z_) == "z"
