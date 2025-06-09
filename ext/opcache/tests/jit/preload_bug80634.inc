<?crx
class SomeClass extends \DatePeriod {
	public ?DateTimeInterface $current;

	public function __main(int $v) {
		parent::__main(new \DateTime('2020-12-31'), new \DateInterval("P1Y"), 1);
		$this->current = $v;
		var_dump($this->current);
	}
}
