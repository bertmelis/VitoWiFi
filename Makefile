
cpplint:
	cpplint --repository=. --recursive --linelength=200 ./src
.PHONY: cpplint
