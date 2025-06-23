#pragma once
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <memory>
#include <deque>

/**
 * @class UndoAction
 * @brief Abstract base class for handling undoable actions.
 *
 * This class provides an interface for implementing undo actions that
 * can be applied to a QGraphicsScene or similar context. Derived classes
 * must implement the undo() method to define specific undo behavior.
 */
class UndoAction {
public:
    virtual ~UndoAction() = default;
    virtual void undo(QGraphicsScene* scene) = 0;
};

class AddItemAction : public UndoAction {
    QGraphicsItem* item;
public:
    explicit AddItemAction(QGraphicsItem* item) : item(item) {}
    void undo(QGraphicsScene* scene) override {
        scene->removeItem(item);
        delete item;
    }
};

/**
 * @class RemoveItemAction
 * @brief Represents an undoable action for removing an item from a QGraphicsScene.
 *
 * RemoveItemAction is a subclass of UndoAction that encapsulates the
 * functionality needed to restore a QGraphicsItem to its original scene.
 * This class is intended to be used as part of an undo/redo system in
 * applications utilizing QGraphicsScene.
 *
 * @inherits UndoAction
 */
class RemoveItemAction : public UndoAction {
    QGraphicsItem* item;
public:
    explicit RemoveItemAction(QGraphicsItem* item) : item(item) {}
    void undo(QGraphicsScene* scene) override {
        scene->addItem(item);
    }
};

/**
 * @class UndoStack
 * @brief A class that manages a stack of undoable actions with a maximum size.
 *
 * The UndoStack class provides functionality to manage a stack of actions
 * that can be undone in reverse order of their addition. It supports adding actions,
 * performing undo operations, and clearing the stack.
 */
class UndoStack {
    std::deque<std::unique_ptr<UndoAction>> stack;
    const size_t maxSize = 20;

public:
    void push(std::unique_ptr<UndoAction> action) {
        if (stack.size() == maxSize)
            stack.pop_front();
        stack.push_back(std::move(action));
    }

    void undo(QGraphicsScene* scene) {
        if (!stack.empty()) {
            stack.back()->undo(scene);
            stack.pop_back();
        }
    }

    void clear() {
        stack.clear();
    }
};
