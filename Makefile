
cpplint:
	cpplint --repository=. --recursive --linelength=200 --filter=-runtime/printf,-build/include,-build/namespace ./src
.PHONY: cpplint
