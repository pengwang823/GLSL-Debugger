/* -*- c++ -*- */
/*
 * Copyright © 2009 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once
#ifndef AST_H
#define AST_H

#include "list.h"
#include "glsl_parser_extras.h"

struct _mesa_glsl_parse_state;

#ifdef AST_DEBUG_STATE
#include "ast_visitor.h"
enum ast_dbg_state {
   ast_dbg_state_unset,
   ast_dbg_state_path, /* path of trace */
   ast_dbg_state_target, /* leaf of trace */
   ast_dbg_state_end
};

enum ast_dbg_state_internal_if {
	ast_dbg_if_unset,             // not debugged so far
	ast_dbg_if_init,              // already visited once
	ast_dbg_if_condition,         // condition in process
	ast_dbg_if_condition_passed,  // condition processed
	ast_dbg_if_then,              // descided to debug true branch
	ast_dbg_if_else,              // descided to debug false branch
	ast_dbg_if_passed             // debugging is past selection
};

enum ast_dbg_state_internal_loop {
	ast_dbg_loop_unset,
	ast_dbg_loop_qyr_init,
	ast_dbg_loop_qyr_test,
	ast_dbg_loop_qyr_terminal,
	ast_dbg_loop_wrk_init,
	ast_dbg_loop_wrk_test,
	ast_dbg_loop_wrk_body,
	ast_dbg_loop_wrk_terminal,
	ast_dbg_loop_select_flow,
	ast_dbg_loop_passed
};

enum ast_dbg_overwrite {
	ast_dbg_ow_unset,
	ast_dbg_ow_original,
	ast_dbg_ow_debug,
	ast_dbg_ow_end
};

enum ast_dbg_sideefects {
	ast_dbg_se_unset = 0,
	ast_dbg_se_general = 1,
	ast_dbg_se_discard = 2,
	ast_dbg_se_emit_vertex = 4
};
#endif

struct YYLTYPE;
struct ast_declarator_list;

/**
 * \defgroup AST Abstract syntax tree node definitions
 *
 * An abstract syntax tree is generated by the parser.  This is a fairly
 * direct representation of the gramma derivation for the source program.
 * No symantic checking is done during the generation of the AST.  Only
 * syntactic checking is done.  Symantic checking is performed by a later
 * stage that converts the AST to a more generic intermediate representation.
 *
 *@{
 */
/**
 * Base class of all abstract syntax tree nodes
 */
class ast_node {
public:
   DECLARE_RALLOC_CXX_OPERATORS(ast_node);

   /**
    * Print an AST node in something approximating the original GLSL code
    */
   virtual void print(void) const;

   /**
    * Convert the AST node to the high-level intermediate representation
    */
   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   /**
    * Retrieve the source location of an AST node
    *
    * This function is primarily used to get the source position of an AST node
    * into a form that can be passed to \c _mesa_glsl_error.
    *
    * \sa _mesa_glsl_error, ast_node::set_location
    */
   struct YYLTYPE get_location(void) const
   {
      struct YYLTYPE locp;

      locp.source = this->location.source;
      locp.first_line = this->location.first_line;
      locp.first_column = this->location.first_column;
      locp.last_line = this->location.last_line;
      locp.last_column = this->location.last_column;

      return locp;
   }

   /**
    * Set the source location of an AST node from a parser location
    *
    * \sa ast_node::get_location
    */
   void set_location(const struct YYLTYPE &locp)
   {
      this->location.source = locp.source;
      this->location.first_line = locp.first_line;
      this->location.first_column = locp.first_column;
      this->location.last_line = locp.last_line;
      this->location.last_column = locp.last_column;
   }

   /**
    * Set the source location range of an AST node using two location nodes
    *
    * \sa ast_node::set_location
    */
   void set_location_range(const struct YYLTYPE &begin, const struct YYLTYPE &end)
   {
      this->location.source = begin.source;
      this->location.first_line = begin.first_line;
      this->location.last_line = end.last_line;
      this->location.first_column = begin.first_column;
      this->location.last_column = end.last_column;
   }

   /**
    * Source location of the AST node.
    */
   struct {
      unsigned source;          /**< GLSL source number. */
      unsigned first_line;      /**< Line number within the source string. */
      unsigned first_column;    /**< Column in the line. */
      unsigned last_line;       /**< Line number within the source string. */
      unsigned last_column;     /**< Column in the line. */
   } location;

   exec_node link;

#ifdef AST_DEBUG_STATE
   virtual class ast_declaration* as_declaration() { return NULL; }
   virtual class ast_parameter_declarator* as_parameter_declarator() { return NULL; }
   virtual class ast_declarator_list* as_declarator_list() { return NULL; }

   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }


   enum ast_dbg_state debug_state;
   enum ast_dbg_overwrite debug_overwrite;
   bool debug_target;
   int debug_sideeffects;
#endif

protected:
   /**
    * The only constructor is protected so that only derived class objects can
    * be created.
    */
   ast_node(void);
};


/**
 * Operators for AST expression nodes.
 */
enum ast_operators {
   ast_assign,
   ast_plus,        /**< Unary + operator. */
   ast_neg,
   ast_add,
   ast_sub,
   ast_mul,
   ast_div,
   ast_mod,
   ast_lshift,
   ast_rshift,
   ast_less,
   ast_greater,
   ast_lequal,
   ast_gequal,
   ast_equal,
   ast_nequal,
   ast_bit_and,
   ast_bit_xor,
   ast_bit_or,
   ast_bit_not,
   ast_logic_and,
   ast_logic_xor,
   ast_logic_or,
   ast_logic_not,

   ast_mul_assign,
   ast_div_assign,
   ast_mod_assign,
   ast_add_assign,
   ast_sub_assign,
   ast_ls_assign,
   ast_rs_assign,
   ast_and_assign,
   ast_xor_assign,
   ast_or_assign,

   ast_conditional,

   ast_pre_inc,
   ast_pre_dec,
   ast_post_inc,
   ast_post_dec,
   ast_field_selection,
   ast_array_index,

   ast_function_call,

   ast_identifier,
   ast_int_constant,
   ast_uint_constant,
   ast_float_constant,
   ast_bool_constant,

   ast_sequence,
   ast_aggregate
};

/**
 * Representation of any sort of expression.
 */
class ast_expression : public ast_node {
public:
   ast_expression(int oper, ast_expression *,
		  ast_expression *, ast_expression *);

   ast_expression(const char *identifier) :
      oper(ast_identifier)
   {
      subexpressions[0] = NULL;
      subexpressions[1] = NULL;
      subexpressions[2] = NULL;
      primary_expression.identifier = identifier;
      this->non_lvalue_description = NULL;
   }

   static const char *operator_string(enum ast_operators op);

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   virtual void print(void) const;

   enum ast_operators oper;

   ast_expression *subexpressions[3];

   union {
      const char *identifier;
      int int_constant;
      float float_constant;
      unsigned uint_constant;
      int bool_constant;
   } primary_expression;


   /**
    * List of expressions for an \c ast_sequence or parameters for an
    * \c ast_function_call
    */
   exec_list expressions;

   /**
    * For things that can't be l-values, this describes what it is.
    *
    * This text is used by the code that generates IR for assignments to
    * detect and emit useful messages for assignments to some things that
    * can't be l-values.  For example, pre- or post-incerement expressions.
    *
    * \note
    * This pointer may be \c NULL.
    */
   const char *non_lvalue_description;
};

class ast_expression_bin : public ast_expression {
public:
   ast_expression_bin(int oper, ast_expression *, ast_expression *);

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual void print(void) const;
};

/**
 * Subclass of expressions for function calls
 */
class ast_function_expression : public ast_expression {
public:
   ast_function_expression(ast_expression *callee)
      : ast_expression(ast_function_call, callee,
		       NULL, NULL),
	cons(false)
   {
      /* empty */
   }

   ast_function_expression(class ast_type_specifier *type)
      : ast_expression(ast_function_call, (ast_expression *) type,
		       NULL, NULL),
	cons(true)
   {
      /* empty */
   }

   bool is_constructor() const
   {
      return cons;
   }

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

private:
   /**
    * Is this function call actually a constructor?
    */
   bool cons;
};

class ast_array_specifier : public ast_node {
public:
   /** Unsized array specifier ([]) */
   explicit ast_array_specifier(const struct YYLTYPE &locp)
     : dimension_count(1), is_unsized_array(true)
   {
      set_location(locp);
   }

   /** Sized array specifier ([dim]) */
   ast_array_specifier(const struct YYLTYPE &locp, ast_expression *dim)
     : dimension_count(1), is_unsized_array(false)
   {
      set_location(locp);
      array_dimensions.push_tail(&dim->link);
   }

   void add_dimension(ast_expression *dim)
   {
      array_dimensions.push_tail(&dim->link);
      dimension_count++;
   }

   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   /* Count including sized and unsized dimensions */
   unsigned dimension_count;

   /* If true, this means that the array has an unsized outermost dimension. */
   bool is_unsized_array;

   /* This list contains objects of type ast_node containing the
    * sized dimensions only, in outermost-to-innermost order.
    */
   exec_list array_dimensions;
};

/**
 * C-style aggregate initialization class
 *
 * Represents C-style initializers of vectors, matrices, arrays, and
 * structures. E.g., vec3 pos = {1.0, 0.0, -1.0} is equivalent to
 * vec3 pos = vec3(1.0, 0.0, -1.0).
 *
 * Specified in GLSL 4.20 and GL_ARB_shading_language_420pack.
 *
 * \sa _mesa_ast_set_aggregate_type
 */
class ast_aggregate_initializer : public ast_expression {
public:
   ast_aggregate_initializer()
      : ast_expression(ast_aggregate, NULL, NULL, NULL),
        constructor_type(NULL)
   {
      /* empty */
   }

   /**
    * glsl_type of the aggregate, which is inferred from the LHS of whatever
    * the aggregate is being used to initialize.  This can't be inferred at
    * parse time (since the parser deals with ast_type_specifiers, not
    * glsl_types), so the parser leaves it NULL.  However, the ast-to-hir
    * conversion code makes sure to fill it in with the appropriate type
    * before hir() is called.
    */
   const glsl_type *constructor_type;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
                          struct _mesa_glsl_parse_state *state);
};

/**
 * Number of possible operators for an ast_expression
 *
 * This is done as a define instead of as an additional value in the enum so
 * that the compiler won't generate spurious messages like "warning:
 * enumeration value ‘ast_num_operators’ not handled in switch"
 */
#define AST_NUM_OPERATORS (ast_sequence + 1)


class ast_compound_statement : public ast_node {
public:
   ast_compound_statement(int new_scope, ast_node *statements);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   int new_scope;
   exec_list statements;
};

class ast_declaration : public ast_node {
public:
   ast_declaration(const char *identifier,
                   ast_array_specifier *array_specifier,
                   ast_expression *initializer);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
   virtual ast_declaration* as_declaration() { return this; }
#endif

   const char *identifier;

   ast_array_specifier *array_specifier;

   ast_expression *initializer;
};


enum {
   ast_precision_none = 0, /**< Absence of precision qualifier. */
   ast_precision_high,
   ast_precision_medium,
   ast_precision_low
};

struct ast_type_qualifier {
   DECLARE_RALLOC_CXX_OPERATORS(ast_type_qualifier);

   union {
      struct {
	 unsigned invariant:1;
	 unsigned constant:1;
	 unsigned attribute:1;
	 unsigned varying:1;
	 unsigned in:1;
	 unsigned out:1;
	 unsigned centroid:1;
         unsigned sample:1;
	 unsigned uniform:1;
	 unsigned smooth:1;
	 unsigned flat:1;
	 unsigned noperspective:1;

	 /** \name Layout qualifiers for GL_ARB_fragment_coord_conventions */
	 /*@{*/
	 unsigned origin_upper_left:1;
	 unsigned pixel_center_integer:1;
	 /*@}*/

	 /**
	  * Flag set if GL_ARB_explicit_attrib_location "location" layout
	  * qualifier is used.
	  */
	 unsigned explicit_location:1;
	 /**
	  * Flag set if GL_ARB_explicit_attrib_location "index" layout
	  * qualifier is used.
	  */
	 unsigned explicit_index:1;

         /**
          * Flag set if GL_ARB_shading_language_420pack "binding" layout
          * qualifier is used.
          */
         unsigned explicit_binding:1;

         /**
          * Flag set if GL_ARB_shader_atomic counter "offset" layout
          * qualifier is used.
          */
         unsigned explicit_offset:1;

         /** \name Layout qualifiers for GL_AMD_conservative_depth */
         /** \{ */
         unsigned depth_any:1;
         unsigned depth_greater:1;
         unsigned depth_less:1;
         unsigned depth_unchanged:1;
         /** \} */

	 /** \name Layout qualifiers for GL_ARB_uniform_buffer_object */
	 /** \{ */
         unsigned std140:1;
         unsigned shared:1;
         unsigned packed:1;
         unsigned column_major:1;
         unsigned row_major:1;
	 /** \} */

	 /** \name Layout qualifiers for GLSL 1.50 geometry shaders */
	 /** \{ */
	 unsigned prim_type:1;
	 unsigned max_vertices:1;
	 /** \} */
      }
      /** \brief Set of flags, accessed by name. */
      q;

      /** \brief Set of flags, accessed as a bitmask. */
      unsigned i;
   } flags;

   /** Precision of the type (highp/medium/lowp). */
   unsigned precision:2;

   /**
    * Location specified via GL_ARB_explicit_attrib_location layout
    *
    * \note
    * This field is only valid if \c explicit_location is set.
    */
   int location;
   /**
    * Index specified via GL_ARB_explicit_attrib_location layout
    *
    * \note
    * This field is only valid if \c explicit_index is set.
    */
   int index;

   /** Maximum output vertices in GLSL 1.50 geometry shaders. */
   int max_vertices;

   /** Input or output primitive type in GLSL 1.50 geometry shaders */
   GLenum prim_type;

   /**
    * Binding specified via GL_ARB_shading_language_420pack's "binding" keyword.
    *
    * \note
    * This field is only valid if \c explicit_binding is set.
    */
   int binding;

   /**
    * Offset specified via GL_ARB_shader_atomic_counter's "offset"
    * keyword.
    *
    * \note
    * This field is only valid if \c explicit_offset is set.
    */
   int offset;

   /**
    * Return true if and only if an interpolation qualifier is present.
    */
   bool has_interpolation() const;

   /**
    * Return whether a layout qualifier is present.
    */
   bool has_layout() const;

   /**
    * Return whether a storage qualifier is present.
    */
   bool has_storage() const;

   /**
    * Return whether an auxiliary storage qualifier is present.
    */
   bool has_auxiliary_storage() const;

   /**
    * \brief Return string representation of interpolation qualifier.
    *
    * If an interpolation qualifier is present, then return that qualifier's
    * string representation. Otherwise, return null. For example, if the
    * noperspective bit is set, then this returns "noperspective".
    *
    * If multiple interpolation qualifiers are somehow present, then the
    * returned string is undefined but not null.
    */
   const char *interpolation_string() const;

   bool merge_qualifier(YYLTYPE *loc,
			_mesa_glsl_parse_state *state,
			ast_type_qualifier q);
};

class ast_declarator_list;

class ast_struct_specifier : public ast_node {
public:
   /**
    * \brief Make a shallow copy of an ast_struct_specifier.
    *
    * Use only if the objects are allocated from the same context and will not
    * be modified. Zeros the inherited ast_node's fields.
    */
   ast_struct_specifier(const ast_struct_specifier& that):
      ast_node(), name(that.name), declarations(that.declarations),
      is_declaration(that.is_declaration)
   {
      /* empty */
   }

   ast_struct_specifier(const char *identifier,
			ast_declarator_list *declarator_list);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   const char *name;
   /* List of ast_declarator_list * */
   exec_list declarations;
   bool is_declaration;
};



class ast_type_specifier : public ast_node {
public:
   /**
    * \brief Make a shallow copy of an ast_type_specifier, specifying array
    *        fields.
    *
    * Use only if the objects are allocated from the same context and will not
    * be modified. Zeros the inherited ast_node's fields.
    */
   ast_type_specifier(const ast_type_specifier *that,
                      ast_array_specifier *array_specifier)
      : ast_node(), type_name(that->type_name), structure(that->structure),
        array_specifier(array_specifier),
        default_precision(that->default_precision)
   {
      /* empty */
   }

   /** Construct a type specifier from a type name */
   ast_type_specifier(const char *name)
      : type_name(name), structure(NULL), array_specifier(NULL),
	default_precision(ast_precision_none)
   {
      /* empty */
   }

   /** Construct a type specifier from a structure definition */
   ast_type_specifier(ast_struct_specifier *s)
      : type_name(s->name), structure(s), array_specifier(NULL),
	default_precision(ast_precision_none)
   {
      /* empty */
   }

   const struct glsl_type *glsl_type(const char **name,
				     struct _mesa_glsl_parse_state *state)
      const;

   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   ir_rvalue *hir(exec_list *, struct _mesa_glsl_parse_state *);

   const char *type_name;
   ast_struct_specifier *structure;

   ast_array_specifier *array_specifier;

   /** For precision statements, this is the given precision; otherwise none. */
   unsigned default_precision:2;
};


class ast_fully_specified_type : public ast_node {
public:

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual void print(void) const;
   bool has_qualifiers() const;

   ast_fully_specified_type() : qualifier(), specifier(NULL)
   {
   }

   const struct glsl_type *glsl_type(const char **name,
				     struct _mesa_glsl_parse_state *state)
      const;

   ast_type_qualifier qualifier;
   ast_type_specifier *specifier;
};


class ast_declarator_list : public ast_node {
public:
   ast_declarator_list(ast_fully_specified_type *);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
   virtual ast_declarator_list* as_declarator_list() { return this; }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   ast_fully_specified_type *type;
   /** List of 'ast_declaration *' */
   exec_list declarations;

   /**
    * Special flag for vertex shader "invariant" declarations.
    *
    * Vertex shaders can contain "invariant" variable redeclarations that do
    * not include a type.  For example, "invariant gl_Position;".  This flag
    * is used to note these cases when no type is specified.
    */
   int invariant;
};


class ast_parameter_declarator : public ast_node {
public:
   ast_parameter_declarator() :
      type(NULL),
      identifier(NULL),
      array_specifier(NULL),
      formal_parameter(false),
      is_void(false)
   {
      /* empty */
   }

   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
   virtual ast_parameter_declarator* as_parameter_declarator() { return this; }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   ast_fully_specified_type *type;
   const char *identifier;
   ast_array_specifier *array_specifier;

   static void parameters_to_hir(exec_list *ast_parameters,
				 bool formal, exec_list *ir_parameters,
				 struct _mesa_glsl_parse_state *state);

private:
   /** Is this parameter declaration part of a formal parameter list? */
   bool formal_parameter;

   /**
    * Is this parameter 'void' type?
    *
    * This field is set by \c ::hir.
    */
   bool is_void;
};


class ast_function : public ast_node {
public:
   ast_function(void);

   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   ast_fully_specified_type *return_type;
   const char *identifier;

   exec_list parameters;

private:
   /**
    * Is this prototype part of the function definition?
    *
    * Used by ast_function_definition::hir to process the parameters, etc.
    * of the function.
    *
    * \sa ::hir
    */
   bool is_definition;

   /**
    * Function signature corresponding to this function prototype instance
    *
    * Used by ast_function_definition::hir to process the parameters, etc.
    * of the function.
    *
    * \sa ::hir
    */
   class ir_function_signature *signature;

   friend class ast_function_definition;
};


class ast_expression_statement : public ast_node {
public:
   ast_expression_statement(ast_expression *);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   ast_expression *expression;
};


class ast_case_label : public ast_node {
public:
   ast_case_label(ast_expression *test_value);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   /**
    * An test value of NULL means 'default'.
    */
   ast_expression *test_value;
};


class ast_case_label_list : public ast_node {
public:
   ast_case_label_list(void);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   /**
    * A list of case labels.
    */
   exec_list labels;
};


class ast_case_statement : public ast_node {
public:
   ast_case_statement(ast_case_label_list *labels);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   ast_case_label_list *labels;

   /**
    * A list of statements.
    */
   exec_list stmts;
};


class ast_case_statement_list : public ast_node {
public:
   ast_case_statement_list(void);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   /**
    * A list of cases.
    */
   exec_list cases;
};


class ast_switch_body : public ast_node {
public:
   ast_switch_body(ast_case_statement_list *stmts);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   ast_case_statement_list *stmts;
};


class ast_selection_statement : public ast_node {
public:
   ast_selection_statement(ast_expression *condition,
			   ast_node *then_statement,
			   ast_node *else_statement);
   virtual void print(void) const;

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
   enum ast_dbg_state_internal_if debug_state_internal;
#endif

   ast_expression *condition;
   ast_node *then_statement;
   ast_node *else_statement;

};


class ast_switch_statement : public ast_node {
public:
   ast_switch_statement(ast_expression *test_expression,
			ast_node *body);
   virtual void print(void) const;

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
   enum ast_dbg_state_internal_if debug_state_internal;
#endif

   ast_expression *test_expression;
   ast_node *body;

protected:
   void test_to_hir(exec_list *, struct _mesa_glsl_parse_state *);
};

class ast_iteration_statement : public ast_node {
public:
   ast_iteration_statement(int mode, ast_node *init, ast_node *condition,
			   ast_expression *rest_expression, ast_node *body);

   virtual void print(void) const;

   virtual ir_rvalue *hir(exec_list *, struct _mesa_glsl_parse_state *);

   enum ast_iteration_modes {
      ast_for,
      ast_while,
      ast_do_while
   } mode;


   ast_node *init_statement;
   ast_node *condition;
   ast_expression *rest_expression;

   ast_node *body;


#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }

   bool need_dbgiter()
   {
      if (this->debug_state_internal != ast_dbg_loop_unset &&
            this->debug_state_internal != ast_dbg_loop_qyr_init &&
            this->debug_state_internal != ast_dbg_loop_wrk_init)
        return true;
      return false;
   }

   enum ast_dbg_state_internal_loop debug_state_internal;
   int debug_iter;
   char* debug_iter_name;
#endif

   /**
    * Generate IR from the condition of a loop
    *
    * This is factored out of ::hir because some loops have the condition
    * test at the top (for and while), and others have it at the end (do-while).
    */
   void condition_to_hir(exec_list *, struct _mesa_glsl_parse_state *);
};


class ast_jump_statement : public ast_node {
public:
   ast_jump_statement(int mode, ast_expression *return_value);
   virtual void print(void) const;

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   enum ast_jump_modes {
      ast_continue,
      ast_break,
      ast_return,
      ast_discard
   } mode;

   ast_expression *opt_return_value;
};


class ast_function_definition : public ast_node {
public:
   ast_function_definition() : prototype(NULL), body(NULL)
   {
   }

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual void print(void) const;

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   ast_function *prototype;
   ast_compound_statement *body;
};

class ast_interface_block : public ast_node {
public:
   ast_interface_block(ast_type_qualifier layout,
                       const char *instance_name,
                       ast_array_specifier *array_specifier)
   : layout(layout), block_name(NULL), instance_name(instance_name),
     array_specifier(array_specifier)
   {
   }

#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

   virtual ir_rvalue *hir(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state);

   ast_type_qualifier layout;
   const char *block_name;

   /**
    * Declared name of the block instance, if specified.
    *
    * If the block does not have an instance name, this field will be
    * \c NULL.
    */
   const char *instance_name;

   /** List of ast_declarator_list * */
   exec_list declarations;

   /**
    * Declared array size of the block instance
    *
    * If the block is not declared as an array or if the block instance array
    * is unsized, this field will be \c NULL.
    */
   ast_array_specifier *array_specifier;
};


/**
 * AST node representing a declaration of the input layout for geometry
 * shaders.
 */
class ast_gs_input_layout : public ast_node
{
public:
   ast_gs_input_layout(const struct YYLTYPE &locp, GLenum prim_type)
      : prim_type(prim_type)
   {
      set_location(locp);
   }

   virtual ir_rvalue *hir(exec_list *instructions,
                          struct _mesa_glsl_parse_state *state);
#ifdef AST_DEBUG_STATE
   virtual void accept(ast_traverse_visitor *v) {  v->visit(this); }
#endif

private:
   const GLenum prim_type;
};

/*@}*/

extern void
_mesa_ast_to_hir(exec_list *instructions, struct _mesa_glsl_parse_state *state);

extern ir_rvalue *
_mesa_ast_field_selection_to_hir(const ast_expression *expr,
				 exec_list *instructions,
				 struct _mesa_glsl_parse_state *state);

extern ir_rvalue *
_mesa_ast_array_index_to_hir(void *mem_ctx,
			     struct _mesa_glsl_parse_state *state,
			     ir_rvalue *array, ir_rvalue *idx,
			     YYLTYPE &loc, YYLTYPE &idx_loc);

extern void
_mesa_ast_set_aggregate_type(const glsl_type *type,
                             ast_expression *expr);

void
emit_function(_mesa_glsl_parse_state *state, ir_function *f);

extern void
check_builtin_array_max_size(const char *name, unsigned size,
                             YYLTYPE loc, struct _mesa_glsl_parse_state *state);

#endif /* AST_H */
