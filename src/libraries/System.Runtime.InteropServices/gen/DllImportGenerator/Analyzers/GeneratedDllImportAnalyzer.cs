using System.Collections.Immutable;
using System.Linq;

using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Diagnostics;

using static Microsoft.Interop.Analyzers.AnalyzerDiagnostics;

namespace Microsoft.Interop.Analyzers
{
    [DiagnosticAnalyzer(LanguageNames.CSharp)]
    public class GeneratedDllImportAnalyzer : DiagnosticAnalyzer
    {
        private const string Category = "Usage";
        
        public readonly static DiagnosticDescriptor GeneratedDllImportMissingModifiers =
            new DiagnosticDescriptor(
                Ids.GeneratedDllImportMissingRequiredModifiers,
                GetResourceString(nameof(Resources.GeneratedDllImportMissingModifiersTitle)),
                GetResourceString(nameof(Resources.GeneratedDllImportMissingModifiersMessage)),
                Category,
                DiagnosticSeverity.Warning,
                isEnabledByDefault: true,
                description: GetResourceString(nameof(Resources.GeneratedDllImportMissingModifiersDescription)));

        public readonly static DiagnosticDescriptor GeneratedDllImportContainingTypeMissingModifiers =
            new DiagnosticDescriptor(
                Ids.GeneratedDllImportContaiingTypeMissingRequiredModifiers,
                GetResourceString(nameof(Resources.GeneratedDllImportContainingTypeMissingModifiersTitle)),
                GetResourceString(nameof(Resources.GeneratedDllImportContainingTypeMissingModifiersMessage)),
                Category,
                DiagnosticSeverity.Warning,
                isEnabledByDefault: true,
                description: GetResourceString(nameof(Resources.GeneratedDllImportContainingTypeMissingModifiersDescription)));

        public override ImmutableArray<DiagnosticDescriptor> SupportedDiagnostics => ImmutableArray.Create(GeneratedDllImportMissingModifiers, GeneratedDllImportContainingTypeMissingModifiers);

        public override void Initialize(AnalysisContext context)
        {
            // Don't analyze generated code
            context.ConfigureGeneratedCodeAnalysis(GeneratedCodeAnalysisFlags.None);
            context.EnableConcurrentExecution();
            context.RegisterCompilationStartAction(
                compilationContext =>
                {
                    INamedTypeSymbol? generatedDllImportAttributeType = compilationContext.Compilation.GetTypeByMetadataName(TypeNames.GeneratedDllImportAttribute);
                    if (generatedDllImportAttributeType == null)
                        return;

                    compilationContext.RegisterSymbolAction(symbolContext => AnalyzeSymbol(symbolContext, generatedDllImportAttributeType), SymbolKind.Method);
                });
        }

        private static void AnalyzeSymbol(SymbolAnalysisContext context, INamedTypeSymbol generatedDllImportAttributeType)
        {
            var methodSymbol = (IMethodSymbol)context.Symbol;

            // Check if method is marked with GeneratedDllImportAttribute
            ImmutableArray<AttributeData> attributes = methodSymbol.GetAttributes();
            if (!attributes.Any(attr => SymbolEqualityComparer.Default.Equals(attr.AttributeClass, generatedDllImportAttributeType)))
                return;

            if (!methodSymbol.IsStatic)
            {
                // Must be marked static
                context.ReportDiagnostic(methodSymbol.CreateDiagnostic(GeneratedDllImportMissingModifiers, methodSymbol.Name));
            }
            else
            {
                // Make sure declarations are marked partial. Technically, we can just check one
                // declaration, since Roslyn would error on inconsistent partial declarations.
                foreach (var reference in methodSymbol.DeclaringSyntaxReferences)
                {
                    var syntax = reference.GetSyntax(context.CancellationToken);
                    if (syntax is MethodDeclarationSyntax methodSyntax && !methodSyntax.Modifiers.Any(SyntaxKind.PartialKeyword))
                    {
                        // Must be marked partial
                        context.ReportDiagnostic(methodSymbol.CreateDiagnostic(GeneratedDllImportMissingModifiers, methodSymbol.Name));
                        break;
                    }
                }

                for (INamedTypeSymbol? typeSymbol = methodSymbol.ContainingType; typeSymbol is not null; typeSymbol = typeSymbol.ContainingType)
                {
                    foreach (var reference in typeSymbol.DeclaringSyntaxReferences)
                    {
                        var syntax = reference.GetSyntax(context.CancellationToken);
                        if (syntax is TypeDeclarationSyntax typeSyntax && !typeSyntax.Modifiers.Any(SyntaxKind.PartialKeyword))
                        {
                            // Must be marked partial
                            context.ReportDiagnostic(typeSymbol.CreateDiagnostic(GeneratedDllImportContainingTypeMissingModifiers, typeSymbol.Name));
                            break;
                        }
                    }
                }
            }
        }
    }
}
