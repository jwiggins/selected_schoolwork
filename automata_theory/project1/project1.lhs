--------------------------
--  CS 341 Project # 1  --
--  John Wiggins    --
--  2/20/2002     --
--------------------------

Goal:

The goal of this project is to design and implement a program that can simulate
FSMs. The simulator should be able to run non-deterministic and deterministic
FSMs, ideally without any assistance from the user.

Description:

  For my solution to this problem, I decided that my interpreter should treat
all machines as non-deterministic. The only difference between the two will be
that deterministic machines are in no more than one state at any time. This
simplified my design quite a bit. One other major design decision was that my
interpreter would only accept machine definitions from a formatted text file.
This was mainly due to the fact that the text file was provided along with the
problem description.
  I chose to implement my solution in Haskell. I did this for a number of
reasons. For one, I was already using Haskell for another CS class this semester.
Also, Haskell is extremely expressive. The parsing and simulation code for
my solution is less than 100 lines of code. (my entire solution w/ comments is
less than 300 lines of code) In fact, this file contains my program. Haskell
allows you to make 'literate' programs where comments are the default and code
is the exception.
  My implementation is broken into three different parts. The first part deals
with parsing the formatted input file. The second part deals with simulation.
The last part ties the first two parts together while also handling user
interaction.
  The parser I wrote for my solution is fairly simplistic. It expects input
with a certain structure and (most likely) fails if certain rules are violated.
The format of the input file is as follows: Each machine definition starts with
a line that consists of "%%". Machine definitions are followed by a line that
consists of "##" and then any number of lines until the next machine definition
starts. These lines are the test strings supplied for each machine. The format
of the machine definition is slightly more forgiving. It must have five or more
lines. The first four lines are the alphabet, states, starting state, and final
state(s). If these lines contain more than one symbol, they must be separated by
commas. Whitespace other than linebreaks is ignored. The fifth through last
lines define the transition function/relation of the machine. Each line must
have three tokens separated by commas. Furthermore, the first and third tokens
must be valid state names for the machine. The second token must be a valid
letter from the alphabet. All parsing is done with little or no error checking.
Errors are caught closer to simulation time by the function aptly named
"checkMachine".
  The simulation code is rather short and sweet. It takes a parsed machine
and a string as input and returns a pretty string denoting success or failure.
Success meaning that the specified machine recognizes the input string.
Simulation consists of stepping through the input string one character at a
time. At each step the epsilon transitions are calculated first and then the
normal transitions are calculated. If the machine is in a state that has no
transition for the given input, then that state "dies". If the machine is
deterministic, this is the end of the line. If it is non-deterministic, then
the machine just has one less state to deal with.s
  Interaction with the program is fairly open ended. The user is first asked
to specify the filename of a formatted input file. This is only done once, so if
the user wishes to run more than one input file, the program must be run again.
Once the input file has been parsed, the user is asked to input a number denoting
the machine they wish to simulate. After that, they are asked to input a number
denoting the input string they wish to test. This happens an arbitrary number of
times The last option at this stage allows the user to enter their own string if
they want to. To leave a stage, the user must input a zero. Upon exiting the
second stage, the user can choose either to simulate another machine or exit the
program.

Implementation:

***************************
*machine parsing functions*
***************************

function lineArray:
- turn a hunk o text into a list of lines
- strips endline characters and blank lines

>lineArray :: String -> String -> [String]
>lineArray tmp []
> | tmp /= "" = (reverse tmp):[]
> | otherwise = []
>lineArray tmp (d:ds)
> | d == '\n' && tmp /= "" = (reverse tmp) : (lineArray "" ds)
> | d == '\n' = lineArray "" ds
> | otherwise = lineArray (d:tmp) ds

function countMachines:
- count the number of machines in a minimally parsed input file
- ie: lineArray -> countMachines -> # of machines

>countMachines :: [String] -> Int
>countMachines ms = (length (filter (\x->x=="%%") ms)) - 1

function selectMachine:
- given a list of lines, return the nth machine def. / test cases pair
- %% denotes the start of a machine definition
- ## denotes the start of test strings

>selectMachine :: Int -> [String] -> ([String],[String])
>selectMachine 0 machs =
> (takeWhile (/= "##") chunk, tail (dropWhile (/= "##") chunk))
> where chunk = (takeWhile (/= "%%") (tail machs))
>selectMachine n machs
> | n > 0 = selectMachine (n-1) (dropWhile (/= "%%") (tail machs))
> | otherwise = error "bad mojo in selectMachine"

type Machine:
* first item is the alphabet
* second item is a list of states
* third item is the starting state
* fourth item is a list of final states
* fifth item is a list of tuples that define the transitions between the states.

>type Machine = ([String],[String],String,[String],[((String,String),String)])

function parseMachine:
- turn a raw machine definition into a slightly more refined Machine tuple

>parseMachine :: [String] -> Machine
>parseMachine def = (a,b,c,d,e)
> where
>   a = parseMachineAlphabet (def !! 0)
>   b = parseMachineStates (def !! 1)
>   c = parseMachineStart (def !! 2)
>   d = parseMachineFinals (def !! 3)
>   e = parseMachineTransitions (drop 4 def)

function parseMachineAlphabet:
- breaks the alphabet line into a list of individual tokens.
- expects a list of comma separated letters (or words)
- checkMachine will ensure that this is a list of single chars

>parseMachineAlphabet :: String -> [String]
>parseMachineAlphabet xs = words (map (\x->if x==',' then ' ' else x) xs)

function parseMachineStates:
- breaks the states line into a list of state names
- expects a comma separated list of words (or letters)

>parseMachineStates :: String -> [String]
>parseMachineStates xs = words (map (\x->if x==',' then ' ' else x) xs)

function parseMachineStart:
- does nothing (returns its argument)
- any bad output from this function will be caught by checkMachine

>parseMachineStart :: String -> String
>parseMachineStart xs = xs

function parseMachineFinals:
- breaks the final states line into a list of state names (1 or more)
- expects a comma separated list of words (or letters)

>parseMachineFinals :: String -> [String]
>parseMachineFinals xs = words (map (\x->if x==',' then ' ' else x) xs)

function parseMachineTransitions:
- turns transition lines into transition tuples
- expects a list of lines of comma separated triples
- checkMachine will verify output

>parseMachineTransitions :: [String] -> [((String,String),String)]
>parseMachineTransitions [] = []
>parseMachineTransitions (xs:xss) =
> ((line !! 0, line !! 1), line !! 2) : parseMachineTransitions xss
> where
>   line = words (map (\x->if x==',' then ' ' else x) xs)

function checkMachine:
- deal with poorly defined machines (syntax checking!)
  * alpha is true if its argument is a list of characters
  * states is true if its argument is a non-empty list
  * start is true if its argument contains a single element from the states list
  * finals is true if its argument contains >1 elements from the states list
  * trans is true if its argument is a list of transitions, where a transition
    has the form ((<state>,<alpha>|epsilon),<state>) 

>checkMachine :: Machine -> Bool
>checkMachine (a,b,c,d,e) =
> (alpha a) && (states b) && (start c) && (finals d) && (trans e)
> where
>   alpha x = and (map (\n->(length n)==1) x)
>   states x = x /= []
>   start x = elem x b
>   finals x = and (map (\n->elem n b) x)
>   trans x =
>     and (map (\((i,j),k)->
>           (elem i b) &&
>           ((elem j a)||j=="epsilon") &&
>           (elem k b)) x)

********************************
* machine simulation functions *
********************************

function removeDupes:
- removes duplicate items from a list

>removeDupes :: Eq a => [a]->[a]
>removeDupes [] = []
>removeDupes (x:xs)
> | not (elem x xs) = x : (removeDupes xs)
> | otherwise = removeDupes xs

function epsilonTransitions:
- handle epsilon transitions
- input a machine and a list of states (1 or more)
- output a list of states (possibly equal to input)

>epsilonTransitions :: Machine -> [String] -> [String]
>epsilonTransitions (_,_,_,_,trans) sts =
> removeDupes
>   (concat (epsilonTr (filter (\((_,t),_)->t=="epsilon") trans) sts))

function epsilonTr:
- build state path list by doing epsilon paths for individual states
- if there are no transition functions, return the states in a list
  (to properly handle DFSMs)

>epsilonTr :: [((String,String),String)] -> [String] -> [[String]]
>epsilonTr _ [] = []
>epsilonTr [] sts = sts:[]
>epsilonTr trans (s:ss) = ((s:[]) : (epsilon trans [] s) : (epsilonTr trans ss))

function epsilon:
- do the actual work of epsilonTr
- adds elements to path if:
  * they aren't already and element
  * they can be reached from an element of path via an epsilon transition

>epsilon :: [((String,String),String)] -> [String] -> String -> [String]
>epsilon trans path curr
> | not (null next) = concat (map (epsilon trans (path++next)) next)
> | otherwise = path
> where next = [n | ((c,_),n)<-trans, c==curr && (not (elem n path))]

function inputTransitions:
- handle normal state change(s) from an input
- input a machine, a list of states, and a character
- output a list of states (possibly empty, due to states 'dying')

>inputTransitions :: Machine -> [String] -> Char -> [String]
>inputTransitions (_,_,_,_,trans) sts c =
> inputTr (filter (\((_,t),_)->t/="epsilon") trans) sts c

function inputTr:
- find all the next states of a machine for a given input
- input a list of transitions, a list of states, and a character
- output a list of states (0 or more)

>inputTr :: [((String,String),String)] -> [String] -> Char -> [String]
>inputTr trans sts c =
> removeDupes [k| s<-sts, ((i,j),k)<-trans, s==i && c==(j!!0)]

function stepMachine:
- step a machine through one character of input.
  ie: do any epsilon transitions, then do normal transitions

>stepMachine :: Machine -> [String] -> Char -> [String]
>stepMachine mach sts c = inputTransitions mach (epsilonTransitions mach sts) c

function walkInput:
- traverse an input string with a machine definition
- input a machine, a list of states, and an input string
- output a list of states

>walkInput :: Machine -> [String] -> String -> [String]
>walkInput _ sts [] = sts
>walkInput mach sts "epsilon" = epsilonTransitions mach sts
>walkInput mach sts (x:xs) = walkInput mach (stepMachine mach sts x) xs

function simulate:
- the whole reason I've been programming all this.
- runs a string through a machine
- input a machine and a string
- output True if the machine accepts the string, False otherwise

>simulate :: Machine -> String -> String
>simulate mach inp
> | numfinal > 0 = "String \"" ++ inp ++ "\" accepted."
> | otherwise = "String \"" ++ inp ++ "\" not accepted."
> where
>   (_,_,st,fin,_) = mach
>   result = walkInput mach (st:[]) inp
>   numfinal = length (filter (\x-> elem x fin) result)

******************************
* user interaction functions *
******************************

function runMachines:
- allows user to select a machine to run from the input file 

>runMachines :: String -> IO ()
>runMachines text =
> do
>   let
>     lines = lineArray "" text
>     count = countMachines lines
>     toInt c = fst ((reads c)!!0)
>     select x = selectMachine x lines
>   putStrLn ("Pick a machine to simulate (" ++
>         (init (concat (map (\x-> (show x)++",") [1..count])))
>         ++ " or 0 to exit):"
>       )
>   m <-getLine
>   if (toInt m) == 0 then return () else
>     do
>       runMachine (select ((toInt m)-1))
>       runMachines text

function runMachine:
- runs a selected machine
- allows user to specify input

>runMachine :: ([String],[String]) -> IO ()
>runMachine (def,tests) =
> do
>   let
>     toInt c = fst ((reads c)!!0)
>     mach = parseMachine def
>     tc = length tests
>     strs =
>       concat (map (\x-> (show (x+1))++") "++(tests!!x)++"\n") [0..(tc-1)])
>   if (checkMachine mach) == False
>     then putStrLn "Selected machine has a syntax error in its definition"
>     else
>     do
>       putStr ("Pick a string to test (or 0 to exit):\n" ++ strs ++
>         (show (tc+1)) ++ ") input your own\n")
>       w <- getLine
>       if (toInt w) == 0 then return ()
>         else
>         do
>           if (toInt w) <= tc then
>             putStrLn (simulate mach (tests !! ((toInt w)-1)))
>             else
>             do
>               str <- getLine
>               putStrLn (simulate mach str)
>           runMachine (def,tests)

function main:
- a function to get things rolling

>main = do
>   putStr "name of input file: "
>   filename <- getLine
>   s <- readFile filename
>   runMachines s
>   putStrLn "Done"
