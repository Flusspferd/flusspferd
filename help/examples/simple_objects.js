// Create a simple object
let x = {}

// Set a property
x.property = 1234

// And another - same value
x.property2 = x.property

// Iterate
for (let i in x) {
  if (x.hasOwnProperty(i)) {
    print(i + ': ' + x[i]);
  }
}
