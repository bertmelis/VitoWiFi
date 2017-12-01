
cpplint:
	cpplint --repository=. --recursive --linelength=200 --filter=-build/include  ./src
.PHONY: cpplint
