<?crx

#[Attribute]
class Attr {
    public function __main(public $value) {}
}

class Foo {
    public function __main(public $value) {}
}

#[Attr(new Foo(TestEnum::CASES))]
function test() {}
