# Contribution Guidelines

Welcome to contribute to this repository! Your help is greatly appreciated and here are some suggestions and guidance to ensure the contribution goes smoothly.

## Contribution mode

You can contribute by:

1. **Reporting problems**： Found an error or improvement in the documentation? Please create an Issue to report the problem.
2. **Propose improvements**： If you have suggestions for this project, send us through an issue.

## Report problems and make recommendations

1. Please create a new Issue in the [Issues](https://github.com/OpenConverterLab/OpenConverter/issues) page.

2. Select the appropriate label, such as "bug" or "enhancement".

3. Provide clear and detailed instructions, including steps to reproduce the problem (if reporting the problem) or your suggestions.

## Submit a Pull Request
Before you submit your Pull Request (PR) consider the following guidelines:
1. Search [GitHub](https://github.com/OpenConverterLab/OpenConverter/pulls) for an open or closed PR that relates to your submission. You don't want to duplicate existing efforts.
2. Please submit an issue instead of PR if you have a better suggestion for format tools. We won't accept a lot of file changes directly without issue statement and assignment.
3. Be sure that the issue describes the problem you're fixing, or documents the design for the feature you'd like to add. Before we accepting your work, we need to conduct some checks and evaluations. So, It will be better if you can discuss the design with us.
4. [Fork](https://docs.github.com/en/github/getting-started-with-github/fork-a-repo) the BabitMF/bmf repo.
5. In your forked repository, make your changes in a new git branch:
    ```
    git checkout -b my-fix-branch main
    ```
6. Create your patch. See our [Patch Guidelines](#patch-guidelines) for details

7. Push your branch to GitHub:
    ```
    git push origin my-fix-branch
    ```
8. In GitHub, send a pull request to `OpenConverter:main` with a clear and unambiguous title.

9. Typically, we review your patch within a week after submission. If the patch involves significant changes, it may take more time. Therefore, if you haven't received any response after a week, please initiate a request for review.

## Contribution Prerequisites
- You are familiar with [Github](https://github.com)
- Maybe you need familiar with [Actions](https://github.com/features/actions)(our default workflow tool).

## Patch Guidelines
- If you add some code inside the framework, please include appropriate test cases
- Commit your changes using a descriptive commit message that follows [AngularJS Git Commit Message Conventions](https://docs.google.com/document/d/1QrDFcIiPjSLDn3EL15IJygNPiHORgU1_OOAqWjiDU5Y/edit).
- Do not commit unrelated changes together.
- Cosmetic changes should be kept in separate patches.
- Follow our [Style Guides](#code-style-guides).

## Code Style Guides

- See [C++ Code clang-format](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format).
  After clang-format installed(version 16.0.6, can be installed by: pip install clang-format==16.0.6), using [tool/format.sh](tool/format.sh).


