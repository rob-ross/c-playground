//
// Created by Rob Ross on 1/29/26.
//

#include "munit/munit.h"
#include "munit/munit_overrides.h"

typedef struct SnekObject snekobject_t;

typedef struct SnekObject {
    char *name;
    snekobject_t *child;
} snekobject_t;

snekobject_t new_node(char *name);


munit_case(RUN, test_new_node_simple, {
  snekobject_t node = new_node("root");
  munit_assert_string_equal(node.name, "root", "Node name should be 'root'");
  munit_assert_null(node.child, "Child should be NULL for a new node");
});

munit_case(RUN, test_new_node_empty_name, {
  snekobject_t node = new_node("");
  munit_assert_string_equal(node.name, "",
                            "Node name should be an empty string");
  munit_assert_null(node.child, "Child should be NULL for a new node");
});

munit_case(SUBMIT, test_new_node_with_child, {
  snekobject_t child = new_node("child");
  snekobject_t parent = new_node("parent");
  parent.child = &child;

  munit_assert_string_equal(parent.name, "parent",
                            "Parent node name should be 'parent'");
  munit_assert_not_null(parent.child, "Parent's child should not be NULL");
  munit_assert_string_equal(parent.child->name, "child",
                            "Child node name should be 'child'");
  munit_assert_null(parent.child->child, "Child's child should be NULL");
});

munit_case(SUBMIT, test_new_node_nested_children, {
  snekobject_t grandchild = new_node("grandchild");
  snekobject_t child = new_node("child");
  snekobject_t parent = new_node("parent");

  child.child = &grandchild;
  parent.child = &child;

  munit_assert_string_equal(parent.name, "parent",
                            "Parent node name should be 'parent'");
  munit_assert_not_null(parent.child, "Parent's child should not be NULL");
  munit_assert_string_equal(parent.child->name, "child",
                            "Child node name should be 'child'");
  munit_assert_not_null(parent.child->child,
                        "Child's child should not be NULL");
  munit_assert_string_equal(parent.child->child->name, "grandchild",
                            "Grandchild node name should be 'grandchild'");
  munit_assert_null(parent.child->child->child,
                    "Grandchild's child should be NULL");
});

int main() {
  MunitTest tests[] = {
      munit_test("/test_new_node_simple", test_new_node_simple),
      munit_test("/test_new_node_empty_name", test_new_node_empty_name),
      munit_test("/test_new_node_with_child", test_new_node_with_child),
      munit_test("/test_new_node_nested_children",
                 test_new_node_nested_children),
      munit_null_test,
  };

  MunitSuite suite = munit_suite("new_node", tests);

  return munit_suite_main(&suite, NULL, 0, NULL);
}