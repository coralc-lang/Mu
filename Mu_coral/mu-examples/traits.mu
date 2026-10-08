#! traits.demo

trait Renderable {
    fn draw(self, target: u32);
    fn label(self) string {
        return "renderable";
    }
}

struct Sprite {
    id: u32;
}

struct Mesh {
    vertices: u32;
}

extend Sprite: trait Renderable {
    fn draw(self, target: u32) {
        paint(self.id);
    }

    fn label(self) string {
        return "sprite";
    }
}

extend Mesh: trait Renderable {
    fn draw(self, target: u32) {
        paint(self.vertices);
    }
}

extend Mesh {
    fn describe(self) string {
        return "mesh";
    }
}

#! @native fn paint(id: u32);

fn render_all(items: Array<dyn Renderable>) {
    for item in items {
        item.draw(0);
        let name = item.label();
    }
}
