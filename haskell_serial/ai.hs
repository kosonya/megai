import Data.String.Utils

commands = ".+-[]<>"

create_programs 1 = [[x] | x <- commands]
create_programs n = [(x:y) | x <- commands, y <- create_programs (n-1) ]

validate_braces_ [] 0 = True
validate_braces_ [] _ = False
validate_braces_ ('[':']':_) _ = False --technically, it's correct, but completley useless
validate_braces_ _ (-1) = False
validate_braces_ ('[':xs) nesting_level = validate_braces_ xs (nesting_level + 1)
validate_braces_ (']':xs) nesting_level = validate_braces_ xs (nesting_level - 1)
validate_braces_ (_:xs) nesting_level = validate_braces_ xs nesting_level
validate_braces xs = validate_braces_ xs 0


main = mapM_ print res
	where
		programs = take 100000000000000000 (create_programs 80)
		res =　filter validate_braces programs
