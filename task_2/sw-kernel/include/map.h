#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...)  EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define __NL__
#define MAP_COMMA __NL__
//Macro for MAP arguments into function declarations
#define MAP_GET_END2()                                              0, MAP_END
#define MAP_GET_END1(...)                                           MAP_GET_END2
#define MAP_GET_END(...)                                            MAP_GET_END1
#define MAP_NEXT0(test, next, ...)                                  next MAP_OUT
#define MAP_NEXT1(test, next)                                       MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next)                                        MAP_NEXT1(MAP_GET_END test, next)
#define MAP_LIST_NEXT1(test, next)                                  MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next)                                   MAP_LIST_NEXT1(MAP_GET_END test, next)
#define MAP_LIST0(f, arg0, arg1, arg2, arg3, function, peak, ...)   f(arg0, arg1, arg2, arg3, function) MAP_COMMA MAP_LIST_NEXT(peak, MAP_LIST1)(f, arg0, arg1, arg2, arg3, peak, __VA_ARGS__)
#define MAP_LIST1(f, arg0, arg1, arg2, arg3, function, peak, ...)   f(arg0, arg1, arg2, arg3, function) MAP_COMMA MAP_LIST_NEXT(peak, MAP_LIST0)(f, arg0, arg1, arg2, arg3, peak, __VA_ARGS__)
#define MAP_LIST(f, arg0, arg1, arg2, arg3, ...)                    EVAL(MAP_LIST1(f, arg0, arg1, arg2, arg3, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

//Define function names
#define SEARCH_BY_KEY_SYNC          search,next,prev,nsm,ngr,nsm_signed,ngr_signed
#define SEARCH_BY_STRUCTURE_SYNC    get_first,get_last,get_first_signed,get_last_signed
#define GET_NUM                     get_num
#define AND_OR_NOT                  or_sync,and_sync,not_sync,or_async,and_async,not_async
#define CUTS                        lseq_sync,ls_sync,greq_sync,gr_sync,lseq_async,ls_async,greq_async,gr_async            
#define DCUTS                       grls_sync,grls_async            
#define INSERT                      ins_sync,ins_async
#define DELETE                      del_sync,del_async       
#define STRUCT_OPERATION            del_str_sync,sq_sync,del_str_async,sq_async     

//Declare handler syntax

#define DECLARE_SEARCH_BY_KEY_SYNC_HANDLES_SYNC(Structure, Key, Value, NotUsed, Function)           [[gnu::always_inline]] Handle< Key , Value > Function ( Key k ) const {return { lnh_ ##Function ( Structure , k )};}
#define DECLARE_SEARCH_BY_STRUCTURE_SYNC_HANDLES_SYNC(Structure, Key, Value, NotUsed, Function)     [[gnu::always_inline]] Handle< Key , Value > Function () const {return { lnh_ ##Function ( Structure )};}
#define DECLARE_NUM_HANDLES(Structure, A, B, C, Function)                                           [[gnu::always_inline]] uint32_t Function () const {return { lnh_ ##Function ( Structure )};}
#define DECLARE_STRUCT_OPERATION_HANDLES(Structure, A, B, C, Function)                              [[gnu::always_inline]] bool Function () const {return { lnh_ ##Function ( Structure )};}
#define DECLARE_AND_OR_NOT_HANDLES(StrA, StrB, StrR, NotUsed, Function)                             [[gnu::always_inline]] bool Function ( uint64_t StrB, uint64_t StrR ) const {return { lnh_ ##Function ( StrA, StrB, StrR )};} 
#define DECLARE_CUTS_HANDLES(Structure, Key, StrR, NotUsed, Function)                               [[gnu::always_inline]] bool Function ( Key k, uint64_t StrR ) const {return { lnh_ ##Function ( k, Structure, StrR )};} 
#define DECLARE_DCUTS_HANDLES(Structure, KeyLS, KeyGR, StrR, Function)                              [[gnu::always_inline]] bool Function ( KeyLS k_ls, KeyGR k_gr, uint64_t StrR ) const {return { lnh_ ##Function ( k_ls, k_gr, Structure, StrR )};} 
#define DECLARE_INSERT_HANDLES(Structure, Key, Value, NotUsed, Function)                            [[gnu::always_inline]] bool Function ( Key k, Value v ) const {return { lnh_ ##Function ( Structure, k, v )};} 
#define DECLARE_DELETE_HANDLES(Structure, Key, Value, NotUsed, Function)                            [[gnu::always_inline]] bool Function ( Key k ) const {return { lnh_ ##Function ( Structure, k )};} 

//define macro for group
#define DECLARE_SEARCH_BY_KEY_SYNC_RESULT_TYPE(Key, Value) \
    MAP_LIST(DECLARE_SEARCH_BY_KEY_SYNC_HANDLES_SYNC,               struct_number,  Key,        Value,      N_U,       SEARCH_BY_KEY_SYNC); 
#define DECLARE_SEARCH_BY_STRUCTURE_SYNC_RESULT_TYPE(Key, Value) \
    MAP_LIST(DECLARE_SEARCH_BY_STRUCTURE_SYNC_HANDLES_SYNC,         struct_number,  Key,        Value,      N_U,       SEARCH_BY_STRUCTURE_SYNC); 
#define DECLARE_GET_NUM_RESULT_TYPE() \
    MAP_LIST(DECLARE_NUM_HANDLES,                                   struct_number,  N_U,        N_U,        N_U,       GET_NUM); 
#define DECLARE_STRUCT_OPERATION_RESULT_TYPE() \
    MAP_LIST(DECLARE_STRUCT_OPERATION_HANDLES,                      struct_number,  N_U,        N_U,        N_U,       STRUCT_OPERATION); 
#define DECLARE_AND_OR_NOT_RESULT_TYPE() \
    MAP_LIST(DECLARE_AND_OR_NOT_HANDLES,                            struct_number,  StrB,       StrR,       N_U,       AND_OR_NOT); 
#define DECLARE_CUTS_RESULT_TYPE(Key) \
    MAP_LIST(DECLARE_CUTS_HANDLES,                                  struct_number,  Key,        StrR,       N_U,       CUTS); 
#define DECLARE_DCUTS_RESULT_TYPE(KeyLS,KeyGR) \
    MAP_LIST(DECLARE_DCUTS_HANDLES,                                 struct_number,  KeyLS,      KeyGR,      StrR,      DCUTS); 
#define DECLARE_INSERT_RESULT_TYPE(Key, Value) \
    MAP_LIST(DECLARE_INSERT_HANDLES,                                struct_number,  Key,        Value,      N_U,       INSERT); 
#define DECLARE_DELETE_RESULT_TYPE(Key) \
    MAP_LIST(DECLARE_DELETE_HANDLES,                                struct_number,  Key,        N_U,        N_U,       DELETE); 

//group macro into macrosets
#define DEFINE_DEFAULT_KEYVAL(Key, Value) \
        DECLARE_SEARCH_BY_KEY_SYNC_RESULT_TYPE(Key,Value) \
        DECLARE_SEARCH_BY_STRUCTURE_SYNC_RESULT_TYPE(Key,Value) \
        DECLARE_GET_NUM_RESULT_TYPE() \
        DECLARE_STRUCT_OPERATION_RESULT_TYPE() \
        DECLARE_AND_OR_NOT_RESULT_TYPE() \
        DECLARE_CUTS_RESULT_TYPE(Key) \
        DECLARE_DCUTS_RESULT_TYPE(Key,Key) \
        DECLARE_INSERT_RESULT_TYPE(Key, Value) \
        DECLARE_DELETE_RESULT_TYPE(Key)

#define DEFINE_KEYVAL(Key, Value) \
        DECLARE_SEARCH_BY_KEY_SYNC_RESULT_TYPE(Key,Value) \
        DECLARE_INSERT_RESULT_TYPE(Key, Value) \
        DECLARE_DELETE_RESULT_TYPE(Key)

#endif
