#pragma once
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <memory>
#include <deque>

class UndoAction {
public:
    virtual ~UndoAction() {}
    virtual void undo(QGraphicsScene* scene) = 0;
};

class AddItemAction : public UndoAction {
    QGraphicsItem* item;
public:
    AddItemAction(QGraphicsItem* item) : item(item) {}
    void undo(QGraphicsScene* scene) override {
        scene->removeItem(item);
        delete item;
    }
};

class RemoveItemAction : public UndoAction {
    QGraphicsItem* item;
public:
    RemoveItemAction(QGraphicsItem* item) : item(item) {}
    void undo(QGraphicsScene* scene) override {
        scene->addItem(item);
    }
};

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
