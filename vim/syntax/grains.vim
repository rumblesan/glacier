" Vim syntax file
" Language: Grains
" Maintainer: Guy John
" Latest Revision: 13 December 2019

if exists("b:current_syntax")
  finish
endif

" Keywords
syntax keyword grainKeyword def var func

" Integer with - + or nothing in front
syntax match grainInteger '\d\+'
syntax match grainInteger '[-+]\d\+'

" Floating point number with decimal no E or e 
syntax match grainFloat '[-+]\d\+\.\d*'

syntax match grainKeyName '[a-zA-Z][a-zA-Z0-9]*:'

syntax match grainMethod '\.[a-zA-Z][a-zA-Z0-9]*'

syntax match grainBlockArgName contained '\v[a-zA-Z][a-zA-Z0-9]*'
syntax region grainBlockArgs start="|" end="|" transparent contains=grainBlockArgName

syntax match grainOperator "\v\*"
syntax match grainOperator "\v/"
syntax match grainOperator "\v\+"
syntax match grainOperator "\v-"
syntax match grainOperator "\v\%"


highlight link grainKeyword Keyword
highlight link grainMethod Function
highlight link grainInteger Number
highlight link grainFloat Float
highlight link grainKeyName Constant
highlight link grainBlockArgName Identifier
highlight link grainOperator Operator

let b:current_syntax = "grains"
