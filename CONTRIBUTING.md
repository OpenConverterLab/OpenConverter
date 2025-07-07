# Contribution Guidelines

Welcome to contribute to this repository! Your help is greatly appreciated and here are some suggestions and guidance to ensure the contribution goes smoothly.

## Contribution mode

You can contribute by:

1. **Reporting problems**： Found an error or improvement in the documentation? Please create an Issue to report the problem.
2. **Propose improvements**： If you have suggestions for this project, send us through an issue.

## Report problems and make recommendations

1. Please create a new Issue in the [Issues](https://github.com/JackLau1222/OpenConverter/issues) page.

2. Select the appropriate label, such as "bug" or "enhancement".

3. Provide clear and detailed instructions, including steps to reproduce the problem (if reporting the problem) or your suggestions.

## Submit Pull Request

1. Fork the repository and clone its main branch locally:

    ```bash
   git clone -b develop https://github.com/your-username/your-repo-name.git
   ```
   
2. Make changes in this directory
   
    
   
4. Commit your changes
    ```bash
    git add .
    git commit -m "Add/modify your-documentation"
    ```
    
5. Push to your forked remote repo
    ```bash
    git push origin feature/your-feature
    ```
    
6. Create a Pull Request (PR) :

- Go to your forked repository and click "New Pull Request".
- Select the target branch to which your changes should be merged.
- Provide clear and concise PR descriptions.
7. We will review your PR and make suggestions or request changes if needed.

## Code Style Guides

- See [C++ Code clang-format](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format).
  After clang-format installed(version 16.0.6, can be installed by: pip install clang-format==16.0.6), using command:
    ```bash
    clang-format -sort-includes=false -style="{BasedOnStyle: llvm, IndentWidth: 4}" -i <your file>
    ```

