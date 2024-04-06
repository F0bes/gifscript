#include <gtest/gtest.h>
#include <memory>
#include <bit>
#include "types.hpp"


TEST(TypeTests, Vec2_Default)
{
	Vec2 vec;
	EXPECT_EQ(vec.x, 0);
	EXPECT_EQ(vec.y, 0);
}

TEST(TypeTests, Vec2_Constructor)
{
	Vec2 vec(1, 2);
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, 2);
}

TEST(TypeTests, Vec2_ParseInteger)
{
	auto vec = Vec2::Parse("1,2");
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, 2);
}

TEST(TypeTests, Vec2_ParseFloat)
{
	auto vec = Vec2::Parse("1.5,2.5");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, std::bit_cast<uint32_t>(2.5f));
}

TEST(TypeTests, Vec2_ParseFloatSuffix)
{
	auto vec = Vec2::Parse("1.5f,2.5f");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, std::bit_cast<uint32_t>(2.5f));
}

TEST(TypeTests, Vec2_ParseHex)
{
	auto vec = Vec2::Parse("0xA,0xB");
	EXPECT_EQ(vec.x, 0xA);
	EXPECT_EQ(vec.y, 0xB);
}

TEST(TypeTests, Vec2_ParseMixed)
{
	auto vec = Vec2::Parse("1,2.5");
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, std::bit_cast<uint32_t>(2.5f));
}

TEST(TypeTests, Vec2_ParseMixed2)
{
	auto vec = Vec2::Parse("1.5,2");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, 2);
}

TEST(TypeTests, Vec2_ParseMixed3)
{
	auto vec = Vec2::Parse("1.5f,2");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, 2);
}

TEST(TypeTests, Vec2_ParseMixed4)
{
	auto vec = Vec2::Parse("1,0xF");
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, 0xF);
}

TEST(TypeTests, Vec2_ParseMixed5)
{
	auto vec = Vec2::Parse("0xF,1");
	EXPECT_EQ(vec.x, 0xF);
	EXPECT_EQ(vec.y, 1);
}

TEST(TypeTests, Vec3_Default)
{
	Vec3 vec;
	EXPECT_EQ(vec.x, 0);
	EXPECT_EQ(vec.y, 0);
	EXPECT_EQ(vec.z, 0);
}

TEST(TypeTests, Vec3_Constructor)
{
	Vec3 vec(1, 2, 3);
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, 2);
	EXPECT_EQ(vec.z, 3);
}

TEST(TypeTests, Vec3_ParseInteger)
{
	auto vec = Vec3::Parse("1,2,3");
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, 2);
	EXPECT_EQ(vec.z, 3);
}

TEST(TypeTests, Vec3_ParseFloat)
{
	auto vec = Vec3::Parse("1.5,2.5,3.5");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, std::bit_cast<uint32_t>(2.5f));
	EXPECT_EQ(vec.z, std::bit_cast<uint32_t>(3.5f));
}

TEST(TypeTests, Vec3_ParseFloatSuffix)
{
	auto vec = Vec3::Parse("1.5f,2.5f,3.5f");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, std::bit_cast<uint32_t>(2.5f));
	EXPECT_EQ(vec.z, std::bit_cast<uint32_t>(3.5f));
}

TEST(TypeTests, Vec3_ParseHex)
{
	auto vec = Vec3::Parse("0xA,0xB,0xC");
	EXPECT_EQ(vec.x, 0xA);
	EXPECT_EQ(vec.y, 0xB);
	EXPECT_EQ(vec.z, 0xC);
}

TEST(TypeTests, Vec3_ParseMixed)
{
	auto vec = Vec3::Parse("1,2.5,3");
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, std::bit_cast<uint32_t>(2.5f));
	EXPECT_EQ(vec.z, 3);
}

TEST(TypeTests, Vec3_ParseMixed2)
{
	auto vec = Vec3::Parse("1.5,2,3");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, 2);
	EXPECT_EQ(vec.z, 3);
}

TEST(TypeTests, Vec3_ParseMixed3)
{
	auto vec = Vec3::Parse("1.5f,2,3.5");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, 2);
	EXPECT_EQ(vec.z, std::bit_cast<uint32_t>(3.5f));
}

TEST(TypeTests, Vec3_ParseMixed4)
{
	auto vec = Vec3::Parse("1,0xF,3");
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, 0xF);
	EXPECT_EQ(vec.z, 3);
}

TEST(TypeTests, Vec3_ParseMixed5)
{
	auto vec = Vec3::Parse("0xF,1,0xF");
	EXPECT_EQ(vec.x, 0xF);
	EXPECT_EQ(vec.y, 1);
	EXPECT_EQ(vec.z, 0xF);
}

TEST(TypeTests, Vec4_Default)
{
	Vec4 vec;
	EXPECT_EQ(vec.x, 0);
	EXPECT_EQ(vec.y, 0);
	EXPECT_EQ(vec.z, 0);
	EXPECT_EQ(vec.w, 0);
}

TEST(TypeTests, Vec4_Constructor)
{
	Vec4 vec(1, 2, 3, 4);
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, 2);
	EXPECT_EQ(vec.z, 3);
	EXPECT_EQ(vec.w, 4);
}

TEST(TypeTests, Vec4_ParseInteger)
{
	auto vec = Vec4::Parse("1,2,3,4");
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, 2);
	EXPECT_EQ(vec.z, 3);
	EXPECT_EQ(vec.w, 4);
}

TEST(TypeTests, Vec4_ParseFloat)
{
	auto vec = Vec4::Parse("1.5,2.5,3.5,4.5");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, std::bit_cast<uint32_t>(2.5f));
	EXPECT_EQ(vec.z, std::bit_cast<uint32_t>(3.5f));
	EXPECT_EQ(vec.w, std::bit_cast<uint32_t>(4.5f));
}

TEST(TypeTests, Vec4_ParseFloatSuffix)
{
	auto vec = Vec4::Parse("1.5f,2.5f,3.5f,4.5f");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, std::bit_cast<uint32_t>(2.5f));
	EXPECT_EQ(vec.z, std::bit_cast<uint32_t>(3.5f));
	EXPECT_EQ(vec.w, std::bit_cast<uint32_t>(4.5f));
}

TEST(TypeTests, Vec4_ParseHex)
{
	auto vec = Vec4::Parse("0xA,0xB,0xC,0xD");
	EXPECT_EQ(vec.x, 0xA);
	EXPECT_EQ(vec.y, 0xB);
	EXPECT_EQ(vec.z, 0xC);
	EXPECT_EQ(vec.w, 0xD);
}

TEST(TypeTests, Vec4_ParseMixed)
{
	auto vec = Vec4::Parse("1,2.5,3,4");
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, std::bit_cast<uint32_t>(2.5f));
	EXPECT_EQ(vec.z, 3);
	EXPECT_EQ(vec.w, 4);
}

TEST(TypeTests, Vec4_ParseMixed2)
{
	auto vec = Vec4::Parse("1.5,2,3,4");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, 2);
	EXPECT_EQ(vec.z, 3);
	EXPECT_EQ(vec.w, 4);
}

TEST(TypeTests, Vec4_ParseMixed3)
{
	auto vec = Vec4::Parse("1.5f,2,3.5,0xB");
	EXPECT_EQ(vec.x, std::bit_cast<uint32_t>(1.5f));
	EXPECT_EQ(vec.y, 2);
	EXPECT_EQ(vec.z, std::bit_cast<uint32_t>(3.5f));
	EXPECT_EQ(vec.w, 0xB);
}

TEST(TypeTests, Vec4_ParseMixed4)
{
	auto vec = Vec4::Parse("1,0xF,3,4.0f");
	EXPECT_EQ(vec.x, 1);
	EXPECT_EQ(vec.y, 0xF);
	EXPECT_EQ(vec.z, 3);
	EXPECT_EQ(vec.w, std::bit_cast<uint32_t>(4.0f));
}

TEST(TypeTests, Vec4_ParseMixed5)
{
	auto vec = Vec4::Parse("0xF,1,0xF,0xE");
	EXPECT_EQ(vec.x, 0xF);
	EXPECT_EQ(vec.y, 1);
	EXPECT_EQ(vec.z, 0xF);
	EXPECT_EQ(vec.w, 0xE);
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
