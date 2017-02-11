#ifndef ASTWALKER_H
#define ASTWALKER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"

#include <map>
#include <iostream>

#include "relations.h"

/*! main class for processing the AST
 */
class RelationsBuilder : public clang::RecursiveASTVisitor<RelationsBuilder>
{
public:
  explicit RelationsBuilder(clang::ASTContext *Context)
    : Context(Context)
    , statements()
  {}

  /* Query functions, they does not recurse into
   * Compound, Loop or Branch, just gets the vars / defs at that level.
   */
  std::set<clang::DeclRefExpr*> vars(clang::Stmt *Stmt);
  std::set<clang::DeclRefExpr*> defs(clang::Stmt *Stmt);

  // we process nodes in Visit*

  // Assign
  bool VisitBinaryOperator(clang::BinaryOperator *Stmt);
  // Compound
  bool VisitCompoundStmt(clang::CompoundStmt *Stmt);
  // Branch
  bool VisitIfStmt(clang::IfStmt *Stmt);
  // Loop
  bool VisitWhileStmt(clang::WhileStmt *Stmt);

private:
  typedef clang::RecursiveASTVisitor<RelationsBuilder> base;
  clang::ASTContext *Context;
  // We store the Statements in a map.
  std::map<clang::Stmt*,std::shared_ptr<Statement>> statements;
};

// necessary class to hook
class RelationsBuilderConsumer : public clang::ASTConsumer {
public:
  explicit RelationsBuilderConsumer(clang::ASTContext *Context)
    : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  RelationsBuilder Visitor;
};

class RelationsBuilderAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new RelationsBuilderConsumer(&Compiler.getASTContext()));
  }
};
#endif // ASTWALKER_H
