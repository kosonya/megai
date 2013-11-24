import Data.String.Utils

commands = "+.-[]<>"

create_programs 1 = [[x] | x <- commands]
create_programs n = [(x:y) | x <- commands, y <- create_programs (n-1) ]

validate_braces_ [] 0 = True
validate_braces_ [] _ = False
validate_braces_ _ (-1) = False
validate_braces_ ('[':xs) nesting_level = validate_braces_ xs (nesting_level + 1)
validate_braces_ (']':xs) nesting_level = validate_braces_ xs (nesting_level - 1)
validate_braces_ (_:xs) nesting_level = validate_braces_ xs nesting_level
validate_braces xs = validate_braces_ xs 0

optimize_program_ [] has_io = has_io
optimize_program_ ('[':']':_) _ = False
optimize_program_ ('+':'-':_) _ = False
optimize_program_ ('-':'+':_) _ = False
optimize_program_ ('<':'>':_) _ = False
optimize_program_ ('>':'<':_) _ = False
optimize_program_ ('.':xs) _ = optimize_program_ xs True
optimize_program_ (x:xs) has_io = optimize_program_ xs has_io
optimize_program xs = optimize_program_ xs False

match_right [] _ _ match_list = match_list
match_right ('[':prog_tail) curpos match_stack match_list = match_right prog_tail (curpos + 1) (curpos:match_stack) (match_list ++ [0])
match_right (']':prog_tail) curpos (stack_head:stack_tail) match_list = match_right prog_tail (curpos + 1) stack_tail (match_list ++ [stack_head])
match_right (_:prog_tail) curpos match_stack match_list = match_right prog_tail (curpos + 1) match_stack (match_list ++ [0])

match_left [] _ match_list = match_list
--match_left (']':prog_tail) curpos (match_head:match_tail) = match_left prog_tail (curpos + 1) (match_tail !! (
--fuck


main = print (match_right ".[.][.[.].].[.[.].[.].]." 0 [] [])

main1 = mapM_ (\x -> (mapM_ print x) )res
	where
		programs = map (\x -> take 100000000000000000 (create_programs x)) [80]
		res =　map (\x -> filter (\y -> (validate_braces y) && (optimize_program y)) x) programs
