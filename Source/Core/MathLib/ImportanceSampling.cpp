//==============================================================================
// Joe Schutte
//==============================================================================

#include "ImportanceSampling.h"
#include <MathLib/Trigonometric.h>
#include <MathLib/FloatStructs.h>
#include <MathLib/FloatFuncs.h>
#include <MathLib/Projection.h>
#include <SystemLib/MemoryAllocation.h>
#include <SystemLib/JsAssert.h>
#include <SystemLib/MinMax.h>

namespace Shooty
{
    namespace ImportanceSampling
    {
        //==============================================================================
        static void SampleDistributionFunction(float* __restrict distribution, uint count, float random01, uint& index, float& pdf)
        {
            // -- binary search the cdf to find the largest sample that is lower than the given random number between 0 and 1
            index = (uint)-1;

            sint low = 0;
            sint high = count - 1;

            while(low <= high) {
                sint mid = (low + high) / 2;

                if(distribution[mid] >= random01) {
                    index = mid;
                    pdf = (mid > 0) ? distribution[mid] - distribution[mid - 1] : distribution[mid];
                    high = mid - 1;
                }
                else {
                    low = mid + 1;
                }
            }

            Assert_(index != (uint)-1);
        }

        //==============================================================================
        uint CalculateMarginalDensityFunctionCount(uint width, uint height)
        {
            return height;
        }

        //==============================================================================
        uint CalculateConditionalDensityFunctionsCount(uint width, uint height)
        {
            return width * height;
        }

        //==============================================================================
        float IblPdf(IblDensityFunctions* distributions, float3 w)
        {
            int32 width  = (int32)distributions->width;
            int32 height = (int32)distributions->height;
            float widthf = (float)width;
            float heightf = (float)height;

            float theta;
            float phi;
            Math::NormalizedCartesianToSpherical(w, theta, phi);

            int32 x = Clamp<int32>((int32)(phi * widthf / Math::TwoPi_ - 0.5f), 0, width);
            int32 y = Clamp<int32>((int32)(theta * heightf / Math::Pi_ - 0.5f), 0, height);

            float mdf = distributions->marginalDensityFunction[y];
            float cdf = (distributions->conditionalDensityFunctions + y * width)[x];

            // -- pdf is probably of x and y sample * sin(theta) to account for the warping along the y axis
            return Math::Sinf(theta) * mdf * cdf;
        }

        //==============================================================================
        void Ibl(IblDensityFunctions* distributions, float r0, float r1, float& theta, float& phi, uint& x, uint& y, float& pdf)
        {
            // - http://www.igorsklyar.com/system/documents/papers/4/fiscourse.comp.pdf Section 4.2
            // - See also: Physically based rendering volume 2 section 13.6.5

            uint width = distributions->width;
            uint height = distributions->height;
            float widthf = (float)width;
            float heightf = (float)height;

            float mdf;
            float cdf;
            SampleDistributionFunction(distributions->marginalDensityFunction, height, r0, y, mdf);
            SampleDistributionFunction(distributions->conditionalDensityFunctions + y * width, width, r1, x, cdf);

            // -- theta represents the vertical position on the sphere and varies between 0 and pi
            theta = (y + 0.5f) * Math::Pi_ / heightf;

            // -- phi represents the horizontal position on the sphere and varies between 0 and 2pi
            phi = (x + 0.5f) * Math::TwoPi_ / widthf;

            // convert from texture space to spherical with the inverse of the Jacobian
            float jacobian = (widthf * heightf) / Math::TwoPi_;

            // -- pdf is probably of x and y sample * sin(theta) to account for the warping along the y axis
            pdf = Math::Sinf(theta) * mdf * cdf * jacobian;
        }

        //==============================================================================
        void ShutdownDensityFunctions(IblDensityFunctions* distributions)
        {
            SafeFree_(distributions->conditionalDensityFunctions);
            SafeFree_(distributions->marginalDensityFunction);
        }

        //==============================================================================
        float GgxDPdf(float roughness, float dotNH)
        {
            float a2 = roughness * roughness;
            float sqrtdenom = (dotNH * dotNH) * (a2 - 1) + 1;

            return a2 / (Math::Pi_ * sqrtdenom * sqrtdenom);
            
        }

        //==============================================================================
        void GgxD(float roughness, float r0, float r1, float& theta, float& phi)
        {
            float m2 = roughness * roughness;

            phi = Math::TwoPi_ * r0;
            theta = Math::Acosf(Math::Sqrtf((1.0f - r1) / ((m2 - 1.0f) * r1 + 1.0f)));
        }

        //==============================================================================
        float BalanceHeuristic(uint nf, float fPdf, uint ng, float gPdf)
        {
            return (nf * fPdf) / (nf * fPdf + ng * gPdf);
        }

        //==============================================================================
        float PowerHeuristic(uint nf, float fPdf, uint ng, float gPdf)
        {
            float f = nf * fPdf;
            float g = ng * gPdf;
            return (f * f) / (f * f + g * g);
        }
    }
}